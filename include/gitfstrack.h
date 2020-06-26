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
	int gid; // provided by fuse
	int uid; // provided by fuse
	time_t time; // time associated to the revision
} gitfs_info;

enum gitfs_object_type {
	GITFS_UNKNOWN,
	GITFS_TREE,
	GITFS_BLOB
};

struct gitfs_object;

void gitfs_dispose(struct gitfs_object * object);

/**
 * Initialize everything
 */
int gitfs_init(const char * repo_path, const char * treeish);

/**
 * Get the object associated with this path
 */
struct gitfs_object * gitfs_get_object(const char * path);

enum gitfs_object_type gitfs_get_type(struct gitfs_object * object);

int gitfs_get_num_entries(struct gitfs_object * object);

/**
 * Return the size of the object. If it is a tree,
 * will return the number of items.
 */
int gitfs_get_size(struct gitfs_object * object);

struct gitfs_object * gitfs_get_tree_entry(struct gitfs_object * tree, int index);

char * gitfs_get_name(struct gitfs_object * object);

/**
 * do not free the pointer
 */
const char * gitfs_get_content(struct gitfs_object * object);

/**
 * Close everything
 */
void gitfs_shutdown();


#endif
