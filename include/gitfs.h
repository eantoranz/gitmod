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
int gitfs_init(const char * repo_path, const char * treeish);

/**
 * Close everything
 */
void gitfs_shutdown();


#endif
