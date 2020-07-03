/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITFS_H
#define GITFS_H

#include <git2.h>

struct gitmod_info {
	const char * treeish; // treeish that is asked to track
	int treeish_is_tree; // will set to 1 if the object that was provided as treeish is a straight tree
	git_repository * repo;
	int gid; // provided by fuse
	int uid; // provided by fuse
	time_t time; // time associated to the revision
} gitmod_info;

enum gitmod_object_type {
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
} gitmod_object;

void gitmod_dispose(gitmod_object * object);

/**
 * Initialize everything
 */
int gitmod_init(const char * repo_path, const char * treeish);

/**
 * Get the object associated with this path
 */
gitmod_object * gitmod_get_object(const char * path, int pull_mode);

int gitmod_get_mode(gitmod_object * object);

enum gitmod_object_type gitmod_get_type(gitmod_object * object);

int gitmod_get_num_entries(gitmod_object * object);

/**
 * Return the size of the object. If it is a tree,
 * will return the number of items.
 */
int gitmod_get_size(gitmod_object * object);

gitmod_object * gitmod_get_tree_entry(gitmod_object * tree, int index, int pull_mode);

char * gitmod_get_name(gitmod_object * object);

/**
 * do not free the pointer
 */
const char * gitmod_get_content(gitmod_object * object);

/**
 * Close everything
 */
void gitmod_shutdown();


#endif
