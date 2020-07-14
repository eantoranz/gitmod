/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

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

