/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <git2.h>
#include <errno.h>
#include "object.h"

enum gitmod_object_type gitmod_object_get_type(gitmod_object * object)
{
	if (!object)
		return GITMOD_OBJECT_UNKNOWN;
	if (object->tree) {
		return GITMOD_OBJECT_TREE;
	}
	if (object->blob) {
		return GITMOD_OBJECT_BLOB;
	}
	return GITMOD_OBJECT_UNKNOWN;
}

int gitmod_object_get_num_entries(gitmod_object * object)
{
	if (!object)
		return -ENOENT;
	
	enum gitmod_object_type type = gitmod_object_get_type(object);
	int res;
	switch (type) {
	case GITMOD_OBJECT_BLOB:
		res = 1;
		break;
	case GITMOD_OBJECT_TREE:
		res = git_tree_entrycount(object->tree);
		break;
	default:
		res = -ENOENT;
	}
	return res;
}

int gitmod_object_get_size(gitmod_object * object)
{
	int res;
	if (!object)
		return -ENOENT;
	if (object->blob)
		res = git_blob_rawsize(object->blob);
	else if (object->tree)
		res = gitmod_object_get_num_entries(object);
	else
		res = -ENOENT;
	
	return res;
}

const char * gitmod_object_get_content(gitmod_object * object)
{
	if (!object)
		return NULL;
	if (!object->blob)
		return NULL;
	return git_blob_rawcontent(object->blob);
}

int gitmod_object_get_mode(gitmod_object * object)
{
	return object ? object->mode : 0;
}

char * gitmod_object_get_name(gitmod_object * object)
{
	return object ? object->name : NULL;
}

void gitmod_object_dispose(gitmod_object ** object)
{
	if (!object)
		return;
	if ((*object)->blob)
		git_blob_free((*object)->blob);
	if ((*object)->name)
		free((*object)->name);
	if ((*object)->path)
		free((*object)->path);

	// finally
	free(*object);
	*object = NULL;
}

