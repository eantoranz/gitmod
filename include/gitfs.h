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
	time_t time; // time associated to the revision
} gitfs_info;

struct gitfs_object;

void gitfs_dispose(struct gitfs_object * object);

/**
 * Initialize everything
 */
int gitfs_init(const char * repo_path, const char * treeish);

/**
 * Get the object associated with this path
 */
int gitfs_get_object(struct gitfs_object ** object, const char * path);

int gitfs_get_num_entries(struct gitfs_object * object);

int gitfs_get_tree_entry(struct gitfs_object ** entry, struct gitfs_object * tree, int index);

char * gitfs_get_name(struct gitfs_object * object);

/**
 * Close everything
 */
void gitfs_shutdown();


#endif
