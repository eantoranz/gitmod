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
	int gid; // provided by fuse
	int uid; // provided by fuse
	time_t time; // time associated to the revision
} gitfs_info;

enum gitfs_object_type {
	GITFS_UNKNOWN,
	GITFS_TREE,
	GITFS_BLOB
};

typedef struct {
	git_tree * tree;
	git_blob * blob;
	char * name; // local name, _not_ fullpath
	char * path; // full path
	int mode;
} gitfs_object;

void gitfs_dispose(gitfs_object * object);

/**
 * Initialize everything
 */
int gitfs_init(const char * repo_path, const char * treeish);

/**
 * Get the object associated with this path
 */
gitfs_object * gitfs_get_object(const char * path, int pull_mode);

int gitfs_get_mode(gitfs_object * object);

enum gitfs_object_type gitfs_get_type(gitfs_object * object);

int gitfs_get_num_entries(gitfs_object * object);

/**
 * Return the size of the object. If it is a tree,
 * will return the number of items.
 */
int gitfs_get_size(gitfs_object * object);

gitfs_object * gitfs_get_tree_entry(gitfs_object * tree, int index, int pull_mode);

char * gitfs_get_name(gitfs_object * object);

/**
 * do not free the pointer
 */
const char * gitfs_get_content(gitfs_object * object);

/**
 * Close everything
 */
void gitfs_shutdown();


#endif
