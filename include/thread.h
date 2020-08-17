/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_THREAD_H
#define GITMOD_THREAD_H

#include "types.h"

/**
 * Crete and start the thread.
 * delay is used in milliseconds. 0 means it's a tight loop
 */
gitmod_thread * gitmod_thread_create(gitmod_info * info, void (*task)(gitmod_thread *), int delay);

void gitmod_thread_set_delay(gitmod_thread * thread, int delay);

void gitmod_thread_release(gitmod_thread ** thread);


#endif
