/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include "root_tree.h"
#include "lock.h"

void gitmod_root_tree_dispose(gitmod_root_tree * root_tree)
{
	gitmod_locker_destroy(root_tree->lock);
	git_tree_free(root_tree->tree);
	free(root_tree);
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
		gitmod_root_tree_dispose(root_tree);
	}
}

