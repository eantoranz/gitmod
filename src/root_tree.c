/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <stdio.h>
#include <string.h>
#include "root_tree.h"
#include "lock.h"

gitmod_root_tree * gitmod_root_tree_create(git_tree * tree, time_t revision_time)
{
	gitmod_root_tree * root_tree;
	root_tree = calloc(1, sizeof(gitmod_root_tree));
	if (root_tree) {
		root_tree->lock = gitmod_locker_create();
		if (root_tree->lock) {
			root_tree->tree = tree;
			root_tree->time = revision_time;
		} else {
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
	gitmod_locker_destroy(&(*root_tree)->lock);
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

gitmod_object * gitmod_root_tree_get_object(gitmod_root_tree * root_tree, const char * path, int pull_mode)
{
	int ret = 0;
	gitmod_object * object = NULL;
	git_tree_entry * tree_entry = NULL;
	if (!root_tree) {
		return NULL;
	}
	if (!(strlen(path) && strcmp(path, "/"))) {
		// root tree
		object = calloc(1, sizeof(gitmod_object));
		object->path = strdup("/");
		object->name = strdup("/");
		object->tree = root_tree->tree;
		if (pull_mode)
			object->mode = 0555; // TODO can we get more info about what the perms are for the mount point?
		return object;
	}

	
	ret = git_tree_entry_bypath(&tree_entry, root_tree->tree, path + (path[0] == '/' ? 1 : 0));
	if (ret) {
		fprintf(stderr, "Could not find the object for the path %s\n", path);
		tree_entry = NULL;
		goto end;
	}
	
	object = gitmod_object_get_from_git_tree_entry(tree_entry, pull_mode);
end:
	if (object)
		object->path = strdup(path);
	if (tree_entry)
		git_tree_entry_free(tree_entry);
	return object;
}
