/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_OBJECT_H
#define GITMOD_OBJECT_H

#include <git2.h>

enum gitmod_object_type {
	GITMOD_OBJECT_UNKNOWN,
	GITMOD_OBJECT_TREE,
	GITMOD_OBJECT_BLOB
};

typedef struct {
	git_tree * tree;
	git_blob * blob;
	char * name; // local name, _not_ fullpath
	char * path; // full path
	int mode;
} gitmod_object;

enum gitmod_object_type gitmod_object_get_type(gitmod_object * object);

int gitmod_object_get_num_entries(gitmod_object * object);

int gitmod_object_get_size(gitmod_object * object);

const char * gitmod_object_get_content(gitmod_object * object);

int gitmod_object_get_mode(gitmod_object * object);

char * gitmod_object_get_name(gitmod_object * object);

void gitmod_object_dispose(gitmod_object ** object);

#endif
