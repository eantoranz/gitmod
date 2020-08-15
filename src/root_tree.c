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
	printf("Disposing of root tree\n");
	if ((*root_tree)->objects_cache) {
#ifdef GITMOD_DEBUG
		printf("Disposing of root tree's object's cache\n");
#endif
		gitmod_cache_dispose(&(*root_tree)->objects_cache);
	}
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
#ifdef GITMOD_DEBUG
	printf("Increasing root tree usage, count is now %d\n", root_tree->usage_counter);
#endif
	gitmod_unlock(root_tree->lock);
}

void gitmod_root_tree_decrease_usage(gitmod_root_tree ** root_tree)
{
	if (!(root_tree && *root_tree))
		return;
	int delete_root_tree;
	gitmod_lock((*root_tree)->lock);
	(*root_tree)->usage_counter--;
#ifdef GITMOD_DEBUG
	printf("Decreasing root tree usage, count is now %d\n", (*root_tree)->usage_counter);
#endif
	delete_root_tree = (*root_tree)->marked_for_deletion && (*root_tree)->usage_counter <= 0;
	gitmod_unlock((*root_tree)->lock);
	if (delete_root_tree) {
		gitmod_root_tree_dispose(root_tree);
	}
}

static gitmod_object * gitmod_root_tree_get_object_from_git_tree_entry(gitmod_info * info, git_tree_entry * git_entry)
{
	gitmod_object * object = calloc(1, sizeof(gitmod_object));
	if (!object)
		return NULL;
	object->mode = git_tree_entry_filemode(git_entry) & 0555; // RO always
	object->name = strdup(git_tree_entry_name(git_entry));
	git_otype otype = git_tree_entry_type(git_entry);
	int ret;
	switch (otype) {
	case GIT_OBJ_BLOB:
		ret = git_tree_entry_to_object((git_object **) &object->blob, info->repo, git_entry);
		break;
	case GIT_OBJ_TREE:
		ret = git_tree_entry_to_object((git_object **) &object->tree, info->repo, git_entry);
		break;
	default:
		ret = -ENOENT;
	}
	if (ret)
		gitmod_object_dispose(&object);
	return object;
}

gitmod_object * gitmod_root_tree_get_object(gitmod_info * info, gitmod_root_tree * root_tree, const char * orig_path)
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
#ifdef GITMOD_DEBUG
	printf("Getting object for path %s\n", path);
#endif
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
#ifdef GITMOD_DEBUG
			printf("Could not find the object for the path %s\n", path);
#endif
			tree_entry = NULL;
			goto end;
		}
		
		object = gitmod_root_tree_get_object_from_git_tree_entry(info, tree_entry);
	}
end:
	if (cached_item && object) {
		gitmod_root_tree_increase_usage(root_tree); // one more item using this root_tree
		gitmod_cache_item_set(cached_item, object); // so that the item can be used
	}
        if (object) {
		if (!object->path)
			object->path = strdup(path);
		if (!object->root_tree)
			object->root_tree = root_tree;
	}
	if (orig_path != path)
		free(path);
	if (tree_entry)
		git_tree_entry_free(tree_entry);
	return object;
}

int gitmod_root_tree_dispose_object(gitmod_object ** object)
{
	if (!(object && *object))
		// nothing to do
		return 0;

#ifdef GITMOD_DEBUG
	printf("Disposing of object for path %s\n", (*object)->path);
#endif
	gitmod_root_tree * root_tree = (*object)->root_tree;
	// if the object is not cached, we can dispose of it direcly
	if (!root_tree->objects_cache) {
		// objects are not cached
		gitmod_object_dispose(object);
		return 0;
	}
	
	// there's some caching involved
	
	gitmod_root_tree_decrease_usage(&root_tree); // this might get rid of EVERYTHING
	if (!root_tree)
		// root_tree has been disposed of (including the objects inside)
		*object = NULL;
	return (!root_tree);
}
