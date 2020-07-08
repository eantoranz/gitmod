/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "root_tree_monitor.h"

static void * root_tree_monitor_cycle(void * params)
{
	gitmod_root_tree_monitor * monitor = params;
	while (monitor->run_root_tree_monitor) {
		monitor->task();
		usleep(monitor->delay * 1000);
	}
	return NULL;
}

gitmod_root_tree_monitor * gitmod_root_tree_monitor_create(void (*task)())
{
	gitmod_root_tree_monitor * monitor = calloc(1, sizeof(gitmod_root_tree_monitor));
	if (monitor) {
		monitor->delay = ROOT_TREEE_MONITOR_DEFAULT_DELAY;
		monitor->task = task;
		monitor->run_root_tree_monitor = 1;
		int res = pthread_create(&monitor->thread, NULL, root_tree_monitor_cycle, monitor);
		if (res) {
			free(monitor);
			monitor = NULL;
		}
	}
	return monitor;
}

void gitmod_root_tree_monitor_release(gitmod_root_tree_monitor * monitor)
{
	if (monitor) {
		monitor->run_root_tree_monitor = 0;
		pthread_join(monitor->thread, NULL);
		free(monitor);
	}
}
