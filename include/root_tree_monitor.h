/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_ROOT_TREE_MONITOR_H
#define GITMOD_ROOT_TREE_MONITOR_H

#include <pthread.h>

#define ROOT_TREEE_MONITOR_DEFAULT_DELAY 100

struct root_tree_monitor_info {
	void (*task)(); // task that will be called during refresh cycle
	pthread_t root_tree_monitor; // pthread instance
	int run_root_tree_monitor;
	int is_running;
	int delay; // delay in milliseconds (0 means it's a tight loop). Default will be set to 100
} root_tree_monitor_info;

/**
 * Start the monitor that will take care of checking the root tree
 */
void gitmod_root_tree_start_monitor(void (*task)());

void gitmod_root_tree_stop_monitor();


#endif
