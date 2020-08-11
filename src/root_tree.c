/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "types.h"
#include "lock.h"
#include "object.h"
#include "gitmod.h"
#include "cache.h"

static int gitmod_tree_walk(const char *root, const git_tree_entry *entry, void *payload)
{
	if (!payload)
		return -1; // stop the walk
	gitmod_cache * cache = payload;
	const char * entry_name = git_tree_entry_name(entry);
	char * path = calloc(1, 1 + strlen(root) + strlen(entry_name) + 1);
	strcat(path, "/");
	strcat(path, root);
	strcat(path, entry_name);
	gitmod_cache_get(cache, path); //map the path to an object
	free(path);
	return 0;
}

static void destroy_cache_key(void * key)
{
	if (key)
		free(key);
}

static void destroy_cache_value(void * value)
{
	gitmod_object ** object = value;
	if (!*object)
		// value hadn't been set yet for this path
		return;
	if ((*object)->lock)
		gitmod_lock((*object)->lock);
	int dispose_of_object = (*object)->usage <= 0;
	if ((*object)->lock)
		gitmod_unlock((*object)->lock);
	if (dispose_of_object)
		gitmod_object_dispose(object);
}

gitmod_root_tree * gitmod_root_tree_create(git_tree * tree, time_t revision_time, int use_cache)
{
	gitmod_root_tree * root_tree;
	root_tree = calloc(1, sizeof(gitmod_root_tree));
	if (root_tree) {
		root_tree->lock = gitmod_locker_create();
		if (root_tree->lock && use_cache) {
			root_tree->objects_cache = gitmod_cache_create(destroy_cache_key, destroy_cache_value);
			// do a walk so that we set all paths right now
			git_tree_walk(tree, GIT_TREEWALK_PRE, gitmod_tree_walk, root_tree->objects_cache);
			// make sure that the root path is associated cause it's not included in the tree walk
			gitmod_cache_get(root_tree->objects_cache, "/");
			gitmod_cache_set_fixed(root_tree->objects_cache, 1);
		}
		if (root_tree->lock && (!use_cache || root_tree->objects_cache)) {
			root_tree->tree = tree;
			root_tree->time = revision_time;
		} else {
			if (root_tree->objects_cache)
				gitmod_cache_dispose(&root_tree->objects_cache);
			if (root_tree->lock)
				gitmod_locker_dispose(&root_tree->lock);
			free(root_tree);
			root_tree = NULL;
		}
	}
	return root_tree;
}

void gitmod_root_tree_dispose(gitmod_root_tree ** root_tree)
{
	if (!(root_tree && *root_tree))
		return;
	if ((*root_tree)->objects_cache)
		gitmod_cache_dispose(&(*root_tree)->objects_cache);
	gitmod_locker_dispose(&(*root_tree)->lock);
	git_tree_free((*root_tree)->tree);
	free(*root_tree);
	*root_tree = NULL;
}

void gitmod_root_tree_increase_usage(gitmod_root_tree * root_tree)
{
	if (!root_tree)
		return;
	gitmod_lock(root_tree->lock);
	root_tree->usage_counter++;
	gitmod_unlock(root_tree->lock);
}

void gitmod_root_tree_decrease_usage(gitmod_root_tree * root_tree)
{
	if (!root_tree)
		return;
	int delete_root_tree;
	gitmod_lock(root_tree->lock);
	root_tree->usage_counter--;
	delete_root_tree = root_tree->marked_for_deletion && root_tree->usage_counter <= 0;
	gitmod_unlock(root_tree->lock);
	if (delete_root_tree) {
		gitmod_root_tree_dispose(&root_tree);
	}
}

static gitmod_object * gitmod_root_tree_get_object_from_git_tree_entry(git_tree_entry * git_entry)
{
	gitmod_object * object = calloc(1, sizeof(gitmod_object));
	if (!object)
		return NULL;
	object->lock = gitmod_locker_create();
	object->mode = git_tree_entry_filemode(git_entry) & 0555; // RO always
	object->name = strdup(git_tree_entry_name(git_entry));
	git_otype otype = git_tree_entry_type(git_entry);
	int ret;
	switch (otype) {
	case GIT_OBJ_BLOB:
		ret = git_tree_entry_to_object((git_object **) &object->blob, gitmod_info.repo, git_entry);
		break;
	case GIT_OBJ_TREE:
		ret = git_tree_entry_to_object((git_object **) &object->tree, gitmod_info.repo, git_entry);
		break;
	default:
		ret = -ENOENT;
	}
	if (ret)
		gitmod_object_dispose(&object);
	return object;
}

gitmod_object * gitmod_root_tree_get_object(gitmod_root_tree * root_tree, const char * orig_path)
{
	int ret = 0;
	gitmod_object * object = NULL;
	git_tree_entry * tree_entry = NULL;
	if (!root_tree) {
		return NULL;
	}
	char * path;
	if (strlen(orig_path) > 0 && orig_path[0] == '/')
		path = (char *) orig_path;
	else {
		path = calloc(1, 1 + strlen(orig_path) + 1);
		strcpy(path, "/");
		strcat(path, orig_path);
	}
	// is the object in memory already?
	gitmod_cache_item * cached_item = NULL;
	if (root_tree->objects_cache) {
		cached_item = gitmod_cache_get(root_tree->objects_cache, path);
		if (!cached_item)
			// this path is not associated
			goto end;
		if (cached_item->content) {
			// the object was already in memory
			object = (gitmod_object *) gitmod_cache_item_get(cached_item);
			goto end;
		}
	}
	if (!(strlen(path) && strcmp(path, "/"))) {
		// root tree
		object = calloc(1, sizeof(gitmod_object));
		object->path = strdup("/");
		object->name = strdup("/");
		object->tree = root_tree->tree;
		object->mode = 0555; // TODO can we get more info about what the perms are for the mount point?
	} else {
		ret = git_tree_entry_bypath(&tree_entry, root_tree->tree, path + (path[0] == '/' ? 1 : 0));
		if (ret) {
			fprintf(stderr, "Could not find the object for the path %s\n", path);
			tree_entry = NULL;
			goto end;
		}
		
		object = gitmod_root_tree_get_object_from_git_tree_entry(tree_entry);
	}
	if (cached_item)
		gitmod_cache_item_set(cached_item, object); // so that the item can be used
end:
        if (object) {
		if (object->lock)
			// make sure no one else is looking
			gitmod_lock(object->lock);
		object->usage++;
		if (!object->path)
			object->path = strdup(path);
		if (!object->root_tree)
			object->root_tree = root_tree;
		if (object->lock)
			gitmod_unlock(object->lock);
	}
	if (orig_path != path)
		free(path);
	if (tree_entry)
		git_tree_entry_free(tree_entry);
	return object;
}

void gitmod_root_tree_dispose_object(gitmod_root_tree * root_tree, gitmod_object ** object)
{
	if (!(object && *object))
		// nothing to do
		return;
	// if the object is not cached, we can dispose of it direcly
	if (!(root_tree && root_tree->objects_cache)) {
		// objects are not cached
		gitmod_object_dispose(object);
		return;
	}
	
	// the object is in cache, but if the tree has changed, we can dispose of this object (_if_ usage == 0)
	if ((*object)->lock)
		gitmod_lock((*object)->lock);
	(*object)->usage--;
	int dispose_of_object = 0;
	// TODO consider if we must request a lock on the root_tree to do the check if the root_tree has changed
	if (root_tree != (*object)->root_tree) {
		// root tree has changed, need to dispose of it
		if ((*object)->usage <= 0)
			dispose_of_object = 1;
	}
	if ((*object)->lock)
		gitmod_unlock((*object)->lock);
	if (dispose_of_object)
		gitmod_object_dispose(object);
}
