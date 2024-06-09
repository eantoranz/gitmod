/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_H
#define GITMOD_H

#include <git2.h>
#include "gitmod/object.h"
#include "gitmod/types.h"
#include "gitmod/lock.h"
#include "gitmod/root_tree.h"
#include "gitmod/thread.h"
#include "gitmod/cache.h"

#define GITMOD_OPTION_FIX 1
#define GITMOD_OPTION_KEEP_IN_MEMORY 1<<1

/**
 * start gitmod library. Will return 0 on success.
 */
int gitmod_init();

/**
 * Stop gitmod
 */
void gitmod_shutdown();

/**
 * start tracking a repo/treeish
 * 
 * root_tree_delay: milliseconds to wait before checking that the treeish has moved. 0 means it's a tight loop
 */
gitmod_info *gitmod_start(const char *repo_path, const char *treeish, int options, int root_tree_delay);

/**
 * stop tracking a repo/treeish
 */
void gitmod_stop(gitmod_info ** info);

/**
 * Do all things needed when root tree changes.
 * 
 * It is assumed that gitmod_info.lock is _locked_.
 * It will be unlocked as soon as the new_tree is set up
 * 
 * This method is published so that we can test what happens when the root tree moves
 * 
 * Will return if the old tree was deleted at this moment
 */
int gitmod_root_tree_changed(gitmod_info * info, gitmod_root_tree * new_tree);

/**
 * Get the object associated with this path
 */
gitmod_object *gitmod_get_object(gitmod_info * info, const char *path);

gitmod_object *gitmod_get_tree_entry(gitmod_info * info, gitmod_object * tree, int index);

/**
 * Will return if the tree associated to the object was deleted
 */
int gitmod_dispose_object(gitmod_object ** object);

int gitmod_get_mode(gitmod_object * object);

enum gitmod_object_type gitmod_get_type(gitmod_object * object);

int gitmod_get_num_entries(gitmod_object * object);

/**
 * Return the size of the object. If it is a tree,
 * will return the number of items.
 */
int gitmod_get_size(gitmod_object * object);

char *gitmod_get_name(gitmod_object * object);

/**
 * do not free the pointer
 */
const char *gitmod_get_content(gitmod_object * object);

#endif
