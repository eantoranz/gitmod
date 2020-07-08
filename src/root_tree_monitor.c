/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <pthread.h>
#include <unistd.h>
#include "root_tree_monitor.h"

static void * root_tree_monitor_cycle()
{
	while (root_tree_monitor_info.run_root_tree_monitor) {
		root_tree_monitor_info.task();
		usleep(root_tree_monitor_info.delay * 1000);
	}
	return NULL;
}

void gitmod_root_tree_start_monitor(void (*task)())
{
	root_tree_monitor_info.task = task;
	root_tree_monitor_info.run_root_tree_monitor = 1;
	pthread_create(&root_tree_monitor_info.root_tree_monitor, NULL, root_tree_monitor_cycle, NULL);
	root_tree_monitor_info.is_running = 1;
}

void gitmod_root_tree_stop_monitor()
{
	if (root_tree_monitor_info.is_running) {
		root_tree_monitor_info.run_root_tree_monitor = 0;
		pthread_join(root_tree_monitor_info.root_tree_monitor, NULL);
		root_tree_monitor_info.is_running = 0;
	}
}
