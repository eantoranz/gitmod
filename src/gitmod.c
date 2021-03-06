/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <git2.h>
#include <unistd.h>
#include <time.h>
#include "gitmod.h"
#include "lock.h"
#include "root_tree.h"
#include "thread.h"

#if LIBGIT2_SOVERSION < 28 // old implementation
#define GIT2_OBJECT_TREE GIT_OBJ_TREE
#else // new implementation
#define GIT2_OBJECT_TREE GIT_OBJECT_TREE
#endif

gitmod_info info;

gitmod_info * gitmod_get_info()
{
	return &info;
}

static git_tree * gitmod_get_tree_from_tag(git_tag * tag, time_t * time)
{
	git_object * target;
	int ret = git_tag_target(&target, tag);
	if (ret) {
		fprintf(stderr, "There was an error trying to get target from signed tag\n");
		return NULL;
	}
	*time = git_commit_time((git_commit *) target);
	git_tree * tree;
	ret = git_commit_tree(&tree, (git_commit *) target);
	if (ret) {
		tree = NULL;
		fprintf(stderr, "There was an error getting tree from signed tag's target revision\n");
	}
	git_object_free(target);
	return tree;
}

/**
 * Try to find the root tree, this will be done every time we want to do operations (allows for branch tracking)
 */
static git_tree * gitmod_get_root_tree(time_t * revision_time)
{
	int ret;
	git_object * treeish = NULL;
	git_tree * root_tree = NULL;
	ret = git_revparse_single(&treeish, info.repo, info.treeish);
	if (ret) {
		fprintf(stderr, "There was error parsing the threeish %s\n", info.treeish);
		return NULL;
	}
	
	git_otype object_type = git_object_type(treeish);
	git_otype tag_target_type;
	switch (object_type) {
	case GIT_OBJ_TREE:
		fprintf(stderr, "Threeish is a tree object straight\n");
		root_tree = (git_tree *) treeish;
		*revision_time = time(NULL);
		info.treeish_type = GIT_OBJ_TREE;
		goto end;
	case GIT_OBJ_COMMIT:
		// business as usual
		break;
	case GIT_OBJ_TAG:
		// signed tag
		// type of object that it points to has to be a commit
		tag_target_type = git_tag_target_type((git_tag *) treeish);
		if (tag_target_type != GIT_OBJ_COMMIT) {
			fprintf(stderr, "Signed tag does not point to a revision\n");
			goto end;
		}
		break;
	default:
		fprintf(stderr, "Treeish provided does not refer to a revision\n");
		goto end;
	}
	info.treeish_type = object_type;
	
	switch(info.treeish_type) {
	case GIT_OBJ_TREE:
		root_tree = (git_tree *) treeish;
		break;
	case GIT_OBJ_TAG:
		root_tree = gitmod_get_tree_from_tag((git_tag *) treeish, revision_time);
		break;
	default:
		ret =  git_commit_tree(&root_tree, (git_commit *) treeish);
		if (ret) {
			fprintf(stderr, "Could not find tree object for the revision\n");
			goto end;
		}
		*revision_time = git_commit_time((git_commit *) treeish);
	}
end:
	if (info.treeish_type != GIT2_OBJECT_TREE)
		git_object_free(treeish);
	
	return root_tree;
}

static void gitmod_root_tree_monitor_task();

int gitmod_init(const char * repo_path, const char * treeish)
{
#ifdef GITMOD_DEBUG
	printf("Starting gitmod (debug compilation)\n");
#else
	printf("Starting gitmod\n");
#endif
	int ret;

	// save the treeish
	info.treeish = treeish;

	git_libgit2_init();
	ret = git_repository_open(&info.repo, repo_path);
	if (ret) {
		// there was an error opening the repository
		fprintf(stderr, "There was an error opening the git repo at %s\n", repo_path);
		goto end;
	}

#ifdef GITMOD_DEBUG
	printf("Successfully opened repo at %s\n", git_repository_commondir(info.repo));
#endif
	time_t revision_time;
	git_tree * git_root_tree = gitmod_get_root_tree(&revision_time);
	if (!git_root_tree) {
		fprintf(stderr, "Could not open root tree for treeish\n");
		return -ENOENT;
	}
	// need to  create a new root_tree instance
	gitmod_root_tree * root_tree = gitmod_root_tree_create(git_root_tree, revision_time, info.keep_in_memory);
	if (!root_tree) {
		fprintf(stderr, "Could not set up root tree instance\n");
		return -ENOENT;
	}
#ifdef GITMOD_DEBUG
	printf("Using tree %s as the root of the mount point\n", git_oid_tostr_s(git_tree_id(root_tree->tree)));
#else
	printf("Gitmod is ready\n");
#endif
	
	info.root_tree = root_tree;
	if (!info.fix) {
		info.lock = gitmod_locker_create();
		if (!info.lock) {
			fprintf(stderr, "Could not create lock for root tree (ran out of memory?)\n");
			return -ENOMEM;
		}
		info.root_tree_monitor = gitmod_thread_create(gitmod_root_tree_monitor_task, info.root_tree_delay);
		if (!info.root_tree_monitor)
			fprintf(stderr, "Could not create root tree monitor. Will be fixed on the starting root tree\n");
	} else {
#ifdef GITMOD_DEBUG
		printf("Root tree will be fixed\n");
#endif
	}
end:
	return ret;
}

gitmod_object * gitmod_get_object(const char *path)
{
	gitmod_object * object = NULL;
	if (!info.root_tree) {
		return NULL;
	}
	// Will make sure to be the only one looking at the root_tree
	object = gitmod_root_tree_get_object(&info, info.root_tree, path);
	return object;
}

gitmod_object * gitmod_get_tree_entry(gitmod_object * tree, int index)
{
	return gitmod_object_get_tree_entry(&info, info.root_tree, tree, index);
}

int gitmod_dispose_object(gitmod_object ** object)
{
	return gitmod_root_tree_dispose_object(object);
}

void gitmod_shutdown()
{
	if (info.root_tree_monitor)
		gitmod_thread_release(&info.root_tree_monitor);
	info.root_tree_monitor = NULL;
	git_repository_free(info.repo);
	if (info.root_tree)
		gitmod_root_tree_dispose(&info.root_tree);
	if (info.lock)
		gitmod_locker_dispose(&info.lock);
	// going out, for the time being
	git_libgit2_shutdown();
}

int gitmod_root_tree_changed(gitmod_root_tree * new_tree)
{
	printf("root tree changed\n");
	gitmod_root_tree * old_tree = info.root_tree;
	info.root_tree = new_tree;
	gitmod_unlock(info.lock);
	gitmod_lock(old_tree->lock);
	// set it for deletion right away
	old_tree->marked_for_deletion = 1;
	int delete_root_tree = old_tree->usage_counter <= 0; // not decreasing, just checking how many resources are out
	gitmod_unlock(old_tree->lock);
	if (delete_root_tree)
		gitmod_root_tree_dispose(&old_tree);
	return delete_root_tree;
}

static void gitmod_root_tree_monitor_task()
{
	time_t revision_time;
	git_tree * new_tree = gitmod_get_root_tree(&revision_time);
	if (new_tree) {
		if (git_oid_cmp(git_tree_id(info.root_tree->tree), git_tree_id(new_tree))) {
			// apparently the tree moved....
			gitmod_lock(info.lock);
			
			gitmod_root_tree * old_tree = info.root_tree;
			
			// now we are the only ones watching the old tree, let's check again
			if (git_oid_cmp(git_tree_id(old_tree->tree), git_tree_id(new_tree)))
				// it did change, indeed
				// we can replace the old tree with the new one and let it run normally
				// this will take care of unlocking
				gitmod_root_tree_changed(gitmod_root_tree_create(new_tree, revision_time, info.keep_in_memory));
			else
				gitmod_unlock(info.lock); // no need to make anybody wait, the new tree can be used from now on
		} else
			git_tree_free(new_tree);
	}
}
