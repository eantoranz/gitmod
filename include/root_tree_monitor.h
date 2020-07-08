/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_ROOT_TREE_MONITOR_H
#define GITMOD_ROOT_TREE_MONITOR_H

#include <pthread.h>

#define ROOT_TREEE_MONITOR_DEFAULT_DELAY 100

typedef struct {
	void (*task)(); // task that will be called during refresh cycle
	pthread_t thread; // pthread instance
	int run_root_tree_monitor;
	int delay; // delay in milliseconds (0 means it's a tight loop). Default will be set to 100
} gitmod_root_tree_monitor;

/**
 * Start the monitor that will take care of checking the root tree
 */
gitmod_root_tree_monitor * gitmod_root_tree_monitor_create(void (*task)());

void gitmod_root_tree_monitor_release(gitmod_root_tree_monitor *);


#endif
