/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_ROOT_TREE_H
#define GITMOD_ROOT_TREE_H

#include "types.h"

#define ROOT_TREEE_MONITOR_DEFAULT_DELAY 100

gitmod_root_tree * gitmod_root_tree_create(git_tree * tree, time_t revision_time, int use_cache);

/*
 * If a call is being made to destroy root tree, it is because we are disposing of the root tree and all of its objects
 */
void gitmod_root_tree_dispose(gitmod_root_tree ** root_tree);

void gitmod_root_tree_increase_usage(gitmod_root_tree * root_tree);

/**
 * Decrease usage. Will get a lock to do the decrease.
 * If the root_tree has been set for deletion _and_ the counter reached 0 on this call, we will free it
 */
void gitmod_root_tree_decrease_usage(gitmod_root_tree ** root_tree);

/**
 * When we call this method, it is assumed the its usage counter has already been increased
 */
gitmod_object * gitmod_root_tree_get_object(gitmod_info * info, gitmod_root_tree * tree, const char * path);

/**
 * Pass in the _current_ root tree.
 * The object's root tree will be asked to decrease its usage
 * 
 * Will return if the object's root_tree was deleted
 */
int gitmod_root_tree_dispose_object(gitmod_object ** object);

#endif

