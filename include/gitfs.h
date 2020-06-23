/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITFS_H
#define GITFS_H

#include <git2.h>

struct gitfs_info {
	const char * treeish; // treeish that is asked to track
	git_repository * repo;
	git_object * revision; // revision that is being tracked // TODO will change
	git_tree * root_tree;
} gitfs_info;

/**
 * Initialize everything
 */
int gitfs_git_init(const char * repo_path, const char * treeish);

/**
 * Given a path, how many items are included in the path?
 */
int gitfs_get_num_items(const char * path);

/**
 * Get the name of the item with that index on the tree with that path
 */
char * gitfs_get_entry_name(const char * tree_path, int index);

/**
 * Close everything
 */
void gitfs_git_shutdown();


#endif
