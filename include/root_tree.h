/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_ROOT_TREE_H
#define GITMOD_ROOT_TREE_H

#include <git2.h>
#include "lock.h"

#define ROOT_TREEE_MONITOR_DEFAULT_DELAY 100

typedef struct {
	git_tree * tree;
	time_t time;
	gitmod_locker * lock;
	int usage_counter;
	int marked_for_deletion;
} gitmod_root_tree;

void gitmod_root_tree_dispose(gitmod_root_tree ** root_tree);

void gitmod_root_tree_increase_usage(gitmod_root_tree * root_tree);

/**
 * Decrease usage. Will get a lock to do the decrease.
 * If the root_tree has been set for deletion _and_ the counter reached 0 on this call, we will free it
 */
void gitmod_root_tree_decrease_usage(gitmod_root_tree * root_tree);

#endif

