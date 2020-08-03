/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_OBJECT_H
#define GITMOD_OBJECT_H

#include <git2.h>
#include "types.h"

enum gitmod_object_type gitmod_object_get_type(gitmod_object * object);

int gitmod_object_get_num_entries(gitmod_object * object);

int gitmod_object_get_size(gitmod_object * object);

const char * gitmod_object_get_content(gitmod_object * object);

int gitmod_object_get_mode(gitmod_object * object);

char * gitmod_object_get_name(gitmod_object * object);

void gitmod_object_dispose(gitmod_object ** object);

gitmod_object * gitmod_object_get_from_git_tree_entry(git_tree_entry * git_entry, int pull_mode);

/**
 * Used on objects that are _trees_
 */
gitmod_object * gitmod_object_get_tree_entry(gitmod_object * tree, int index, int pull_mode);

#endif
