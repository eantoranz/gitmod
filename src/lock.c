/**
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv3
 */

#include <pthread.h>
#include "lock.h"

void gitmod_lock(gitmod_locker * locker)
{
	pthread_mutex_lock(&locker->lock);
}

void gitmod_unlock(gitmod_locker * locker)
{
	pthread_mutex_unlock(&locker->lock);
}

