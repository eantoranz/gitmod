/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_H
#define GITMOD_H

#include <git2.h>
#include "object.h"
#include "types.h"
#include "lock.h"
#include "root_tree.h"
#include "thread.h"

gitmod_info * gitmod_get_info();

/**
 * Initialize everything
 */
int gitmod_init(const char * repo_path, const char * treeish);

/**
 * Get the object associated with this path
 */
gitmod_object * gitmod_get_object(const char * path);

gitmod_object * gitmod_get_tree_entry(gitmod_object * tree, int index);

void gitmod_dispose_object(gitmod_object ** object);

int gitmod_get_mode(gitmod_object * object);

enum gitmod_object_type gitmod_get_type(gitmod_object * object);

int gitmod_get_num_entries(gitmod_object * object);

/**
 * Return the size of the object. If it is a tree,
 * will return the number of items.
 */
int gitmod_get_size(gitmod_object * object);

gitmod_object * gitmod_get_tree_entry(gitmod_object * tree, int index);

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
