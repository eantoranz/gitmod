/**
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv3
 */

#ifndef GITMOD_LOCK_H
#define GITMOD_LOCK_H

#include <pthread.h>

typedef struct {
	pthread_mutex_t lock;
} gitmod_locker;

gitmod_locker * gitmod_locker_create();

void gitmod_lock(gitmod_locker * locker);

void gitmod_unlock(gitmod_locker * locker);

void gitmod_locker_dispose(gitmod_locker ** locker);

#endif
