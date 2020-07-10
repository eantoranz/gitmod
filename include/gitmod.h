/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_H
#define GITMOD_H

#include <git2.h>
#include "object.h"
#include "lock.h"
#include "root_tree.h"
#include "thread.h"

struct gitmod_info {
	git_repository * repo;
	const char * treeish; // treeish that is asked to track
	git_otype treeish_type;
	gitmod_root_tree * root_tree;
	int gid; // provided by fuse
	int uid; // provided by fuse
	gitmod_locker * lock;
	gitmod_thread * root_tree_monitor;
	int root_tree_delay; // in milliseconds (0 is a tight loop)
	int fix; // use to not track changes in root tree
} gitmod_info;

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
