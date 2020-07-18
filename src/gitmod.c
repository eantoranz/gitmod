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
static gitmod_root_tree * gitmod_get_root_tree() {
	int ret;
	git_object * treeish = NULL;
	git_tree * root_tree = NULL;
	gitmod_root_tree * root_tree_node = NULL;
	time_t revision_time;
	ret = git_revparse_single(&treeish, gitmod_info.repo, gitmod_info.treeish);
	if (ret) {
		fprintf(stderr, "There was error parsing the threeish %s\n", gitmod_info.treeish);
		return NULL;
	}
	
	git_otype object_type = git_object_type(treeish);
	git_otype tag_target_type;
	switch (object_type) {
	case GIT_OBJ_TREE:
		fprintf(stderr, "Threeish is a tree object straight\n");
		root_tree = (git_tree *) treeish;
		revision_time = time(NULL);
		gitmod_info.treeish_type = GIT_OBJ_TREE;
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
	gitmod_info.treeish_type = object_type;
	
	switch(gitmod_info.treeish_type) {
	case GIT_OBJ_TREE:
		root_tree = (git_tree *) treeish;
		break;
	case GIT_OBJ_TAG:
		root_tree = gitmod_get_tree_from_tag((git_tag *) treeish, &revision_time);
		break;
	default:
		ret =  git_commit_tree(&root_tree, (git_commit *) treeish);
		if (ret) {
			fprintf(stderr, "Could not find tree object for the revision\n");
			goto end;
		}
		revision_time = git_commit_time((git_commit *) treeish);
	}
end:
	if (root_tree) {
		root_tree_node = gitmod_root_tree_create(root_tree, revision_time);
		if (!root_tree_node)
			fprintf(stderr, "Could not create a gitmod root tree instance for git's root tree\n");
	}
	if (gitmod_info.treeish_type != GIT2_OBJECT_TREE)
		git_object_free(treeish);
	
	return root_tree_node;
}

static void gitmod_root_tree_monitor_task();

int gitmod_init(const char * repo_path, const char * treeish)
{
	int ret;

	// save the treeish
	gitmod_info.treeish = treeish;

	git_libgit2_init();
	ret = git_repository_open(&gitmod_info.repo, repo_path);
	if (ret) {
		// there was an error opening the repository
		fprintf(stderr, "There was an error opening the git repo at %s\n", repo_path);
		goto end;
	}

	printf("Successfully opened repo at %s\n", git_repository_commondir(gitmod_info.repo));
	gitmod_root_tree * root_tree = gitmod_get_root_tree();
	if (!root_tree) {
		fprintf(stderr, "Could not open root tree for treeish");
		return -ENOENT;
	}
	
	printf("Using tree %s as the root of the mount point\n", git_oid_tostr_s(git_tree_id(root_tree->tree)));
	
	gitmod_info.root_tree = root_tree;
	if (!gitmod_info.fix) {
		gitmod_info.lock = gitmod_locker_create();
		if (!gitmod_info.lock) {
			fprintf(stderr, "Could not create lock for root tree (ran out of memory?)\n");
			return -ENOMEM;
		}
		gitmod_info.root_tree_monitor = gitmod_thread_create(gitmod_root_tree_monitor_task, gitmod_info.root_tree_delay);
		if (!gitmod_info.root_tree_monitor)
			fprintf(stderr, "Could not create root tree monitor. Will be fixed on the starting root tree\n");
	} else
		printf("Root tree will be fixed\n");
end:
	return ret;
}

gitmod_object * gitmod_get_object(const char *path, int pull_mode)
{
	gitmod_object * object = NULL;
	if (!gitmod_info.root_tree) {
		return NULL;
	}
	// Will make sure to be the only one looking at the root_tree
	gitmod_lock(gitmod_info.lock);
	gitmod_root_tree_increase_usage(gitmod_info.root_tree); // it will get a lock and release it for increasign counter
	gitmod_unlock(gitmod_info.lock);
	object = gitmod_root_tree_get_object(gitmod_info.root_tree, path, pull_mode);
	gitmod_root_tree_decrease_usage(gitmod_info.root_tree);
	return object;
}

void gitmod_shutdown()
{
	if (gitmod_info.root_tree_monitor)
		gitmod_thread_release(&gitmod_info.root_tree_monitor);
	gitmod_info.root_tree_monitor = NULL;
	git_repository_free(gitmod_info.repo);
	if (gitmod_info.lock)
		gitmod_locker_destroy(&gitmod_info.lock);
	// going out, for the time being
	git_libgit2_shutdown();
}

static void gitmod_root_tree_monitor_task()
{
	gitmod_root_tree * new_tree = gitmod_get_root_tree();
	if (new_tree) {
		if (git_oid_cmp(git_tree_id(gitmod_info.root_tree->tree), git_tree_id(new_tree->tree))) {
			// apparently the tree moved....
			gitmod_lock(gitmod_info.lock);
			
			gitmod_root_tree * old_tree = gitmod_info.root_tree;
			
			// now we are the only one watching the old tree
			if (git_oid_cmp(git_tree_id(old_tree->tree), git_tree_id(new_tree->tree))) {
				printf("root tree changed\n");
				// it did change, indeed
				// we can replace the old tree with the new one and let it run normally
				gitmod_info.root_tree = new_tree;
			}
			
			gitmod_unlock(gitmod_info.lock); // no need to make anybody wait, the new tree can be used from now on
			
			if (old_tree != new_tree) {
				gitmod_lock(old_tree->lock);
				// set if for deletion right away
				old_tree->marked_for_deletion = 1;
				int delete_root_tree = old_tree->usage_counter == 0;
				gitmod_unlock(old_tree->lock);
				if (delete_root_tree)
				gitmod_root_tree_dispose(&old_tree);
			}
		} else
			gitmod_root_tree_dispose(&new_tree);
	}
}
