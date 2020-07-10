/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_THREAD_H
#define GITMOD_THREAD_H

#include <pthread.h>


typedef struct {
	void (*task)(); // task that will be called during refresh cycle
	pthread_t thread; // pthread instance
	int run_thread;
	int delay; // delay in milliseconds (0 means it's a tight loop). Default will be set to 100
} gitmod_thread;

/**
 * Crete and start the thread.
 * delay is used in milliseconds. 0 means it's a tight loop
 */
gitmod_thread * gitmod_thread_create(void (*task)(), int delay);

void gitmod_thread_set_delay(gitmod_thread * thread, int delay);

void gitmod_thread_release(gitmod_thread ** thread);


#endif
