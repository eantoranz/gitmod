/**
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv3
 */

#include <pthread.h>
#include <stdlib.h>
#include "gitmod/types.h"

gitmod_locker * gitmod_locker_create()
{
	gitmod_locker * locker = calloc(1, sizeof(gitmod_locker));
	if (locker)
		pthread_mutex_init(&locker->lock, NULL);
	return locker;
}

void gitmod_lock(gitmod_locker * locker)
{
	if (locker)
		pthread_mutex_lock(&locker->lock);
}

void gitmod_unlock(gitmod_locker * locker)
{
	if (locker)
		pthread_mutex_unlock(&locker->lock);
}

void gitmod_locker_dispose(gitmod_locker ** locker)
{
	if (!(locker && *locker))
		return;
	pthread_mutex_destroy(&(*locker)->lock);
	free(*locker);
	*locker = NULL;
}
