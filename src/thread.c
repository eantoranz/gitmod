/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "thread.h"

static void * thread_task(void * params)
{
	gitmod_thread * thread = params;
	while (thread->run_thread) {
		thread->task();
		usleep(thread->delay * 1000); // sleep for a millisecond (or 10) and loop over so that it doesn't _hang_ (like when closing the application)
	}
	return NULL;
}

gitmod_thread * gitmod_thread_create(void (*task)(), int delay)
{
	gitmod_thread * thread = calloc(1, sizeof(gitmod_thread));
	if (thread) {
		thread->delay = delay;
		thread->task = task;
		thread->run_thread = 1;
		int res = pthread_create(&thread->thread, NULL, thread_task, thread);
		if (res) {
			free(thread);
			thread = NULL;
		}
	}
	return thread;
}

void gitmod_thread_set_delay(gitmod_thread * thread, int delay)
{
	thread->delay = delay;
}

void gitmod_thread_release(gitmod_thread ** thread)
{
	if (*thread) {
		(*thread)->run_thread = 0;
		pthread_join((*thread)->thread, NULL);
		free(*thread);
		*thread = NULL;
	}
}
