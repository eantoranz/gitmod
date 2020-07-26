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

gitmod_root_tree * gitmod_root_tree_create(git_tree * tree, time_t revision_time, int use_cache)
{
	gitmod_root_tree * root_tree;
	root_tree = calloc(1, sizeof(gitmod_root_tree));
	if (root_tree) {
		root_tree->lock = gitmod_locker_create();
		if (root_tree->lock && use_cache)
			root_tree->objects_cache = gitmod_cache_create();
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

static gitmod_object * gitmod_root_tree_get_object_from_git_tree_entry(git_tree_entry * git_entry, int pull_mode)
{
	gitmod_object * object = calloc(1, sizeof(gitmod_object));
	if (!object)
		return NULL;
	if (pull_mode)
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

gitmod_object * gitmod_root_tree_get_object(gitmod_root_tree * root_tree, const char * path, int pull_mode)
{
	int ret = 0;
	gitmod_object * object = NULL;
	git_tree_entry * tree_entry = NULL;
	if (!root_tree) {
		return NULL;
	}
	// is the object in memory already?
	gitmod_cache_item * cached_item = NULL;
	if (root_tree->objects_cache) {
		cached_item = gitmod_cache_get(root_tree->objects_cache, path);
		if (cached_item && cached_item->content)
			// the object was already in memory
			return (gitmod_object *) gitmod_cache_item_get(cached_item);
	}
	if (!(strlen(path) && strcmp(path, "/"))) {
		// root tree
		object = calloc(1, sizeof(gitmod_object));
		object->path = strdup("/");
		object->name = strdup("/");
		object->tree = root_tree->tree;
		if (pull_mode)
			object->mode = 0555; // TODO can we get more info about what the perms are for the mount point?
	} else {
		ret = git_tree_entry_bypath(&tree_entry, root_tree->tree, path + (path[0] == '/' ? 1 : 0));
		if (ret) {
			fprintf(stderr, "Could not find the object for the path %s\n", path);
			tree_entry = NULL;
			goto end;
		}
		
		object = gitmod_root_tree_get_object_from_git_tree_entry(tree_entry, pull_mode);
	}
	if (cached_item)
		gitmod_cache_item_set(cached_item, object); // so that the item can be used
end:
	if (object)
		object->path = strdup(path);
	if (tree_entry)
		git_tree_entry_free(tree_entry);
	return object;
}

void gitmod_root_tree_dispose_object(gitmod_root_tree * tree, gitmod_object ** object)
{
	if (!(tree && tree->objects_cache))
		// objects are not cached
		gitmod_object_dispose(object);
	
	// objects are cached so won't dispose of it
}
