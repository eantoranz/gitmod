/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#ifndef GITMOD_CACHE_H
#define GITMOD_CACHE_H

#include <types.h>

gitmod_cache * gitmod_cache_create();

/**
 * Try to find the object for this id.
 * If the object does not exist, a new LOCKED instance of container will be provided,
 * the caller will have to set the content and then unlock it so that the content can be read from other threads.
 * The caller can know if the object has to be filled with content by checking empty (this will be set to true when setting the content)
 * 
 * Will return NULL if there is an error
 */
gitmod_cache_item * gitmod_cache_get(gitmod_cache * cache, const char * id);

int gitmod_cache_size(gitmod_cache * cache);

const void * gitmod_cache_item_get(gitmod_cache_item * item);

/**
 * The object used here _won't_ be duplicate. The caller is in charge of deleting it (TODO: ....after it has been removed from the cache, which is not supported at the time)
 */
void gitmod_cache_item_set(gitmod_cache_item * item, const void * content);

void gitmod_cache_dispose(gitmod_cache ** cache);

#endif
