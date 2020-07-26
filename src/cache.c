/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <stdio.h>
#include <glib.h>
#include "cache.h"
#include "lock.h"

gitmod_cache * gitmod_cache_create()
{
	gitmod_locker * locker = NULL;
	GHashTable * items = NULL;
	gitmod_cache * cache = NULL;
	locker = gitmod_locker_create();
	if (!locker) {
		fprintf(stderr, "Could not set up locker for cache\n");
		goto end;
	}
	
	items = g_hash_table_new(g_str_hash, g_str_equal);
	if (!items) {
		fprintf(stderr, "Could not setup hashtable for cache\n");
		goto end;
	}
end:
	if (locker && items) {
		cache = calloc(1, sizeof(gitmod_cache));
		if (cache) {
			cache->locker = locker;
			cache->items = items;
		}
	}
	if (!cache) {
		// let's revert what we have done
		if (locker)
			gitmod_locker_dispose(&locker);
		if (items)
			g_hash_table_destroy(items);
	}
	
	return cache;
}

gitmod_cache_item * gitmod_cache_get(gitmod_cache * cache, const char * id)
{
	if (!cache)
		return NULL;
	gitmod_cache_item * item = g_hash_table_lookup(cache->items, id);
	if (item)
		return item;
	gitmod_lock(cache->locker);
	// now I am the only one looking into the hash table. Let's tru again
	item = g_hash_table_lookup(cache->items, id);
	if (item) {
		 // another thread had set it up before us. Let's move on
		gitmod_unlock(cache->locker);
		return item;
	}
	
	// need to create a new instance of a container
	item = calloc(1, sizeof(gitmod_cache_item));
	if (item) {
		item->locker = gitmod_locker_create();
		if (!item->locker)
			// can't work with container if it does not have a lock
			free(item);
		else {
			gitmod_lock(item->locker); // so that no other thread can get its content
			
			// we associate the value in the hash table
			g_hash_table_insert(cache->items, strdup(id), item);
		}
	}
	gitmod_unlock(cache->locker);
	return item;
}

int gitmod_cache_size(gitmod_cache * cache)
{
	if (!cache)
		return 0;
	return g_hash_table_size(cache->items);
}

void gitmod_cache_item_set(gitmod_cache_item * item, const void * content)
{
	if (!item)
		return;
	if (!item->content) {
		// content hadn't been set up
		item->content = content;
		gitmod_unlock(item->locker);
	}
}

const void * gitmod_cache_item_get(gitmod_cache_item * item)
{
	if (!item)
		return NULL;
	if (item->content)
		return item->content;
	gitmod_lock(item->locker); // need to lock it so that we wait until content hs been set
	// now we are sure that content has been set
	gitmod_unlock(item->locker);
	return item->content;
}

void gitmod_cache_dispose(gitmod_cache ** cache)
{
	if (*cache) {
		printf("Disposing of cache\n");
		// TODO clear up the objects in memory
		gitmod_locker_dispose(&(*cache)->locker);
		g_hash_table_destroy((*cache)->items); // TODO need to go through the items and dispose of them
		free(*cache);
		*cache = NULL;
	}
}
