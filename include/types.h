/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef TYPES_H
#define TYPES_H

#include <git2.h>
#include <pthread.h>

enum gitmod_object_type {
	GITMOD_OBJECT_UNKNOWN,
	GITMOD_OBJECT_TREE,
	GITMOD_OBJECT_BLOB
};

typedef struct {
	void (*task)(); // task that will be called during refresh cycle
	pthread_t thread; // pthread instance
	int run_thread;
	int delay; // delay in milliseconds (0 means it's a tight loop). Default will be set to 100
} gitmod_thread;

typedef struct {
	pthread_mutex_t lock;
} gitmod_locker;

typedef struct {
	git_tree * tree;
	git_blob * blob;
	char * name; // local name, _not_ fullpath
	char * path; // full path
	int mode;
} gitmod_object;

typedef struct {
	git_tree * tree;
	time_t time;
	gitmod_locker * lock;
	int usage_counter;
	int marked_for_deletion;
} gitmod_root_tree;

#endif