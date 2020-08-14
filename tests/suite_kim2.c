/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 * 
 * Suite keep in memory
 */

#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include "gitmod.h"

static void suitekim_testPullTwoDifferentObjectsMarkForDeletionDisposeOfThem()
{
	git_libgit2_init();
	int ret = git_repository_open(&gitmod_info.repo, ".");
	CU_ASSERT(!ret);
	if (!ret) {
		git_object * treeish;
		ret = git_revparse_single(&treeish, gitmod_info.repo, "81c15dc513f285e727e6e498d24474a885b7dc01"); // tree from v0.7
		CU_ASSERT(!ret);
		if (!ret) {
			gitmod_root_tree * root_tree = gitmod_root_tree_create((git_tree *) treeish, 0, 1);
			CU_ASSERT(root_tree != NULL);
			int root_tree_deleted = 0;
			if (root_tree) {
				CU_ASSERT(root_tree->usage_counter == 0);
				gitmod_object * object = gitmod_root_tree_get_object(root_tree, "/Makefile");
				CU_ASSERT(object != NULL);
				CU_ASSERT(root_tree->usage_counter = 2);
				gitmod_object * object2 = gitmod_root_tree_get_object(root_tree, "/src/gitmod.c");
				CU_ASSERT(object2 != NULL);
				CU_ASSERT(root_tree->usage_counter = 2);
				// we mark it to be disposed
				root_tree->marked_for_deletion=1;
				if (object != NULL) {
					ret = gitmod_root_tree_dispose_object(&object);
					CU_ASSERT(!ret);
					CU_ASSERT(object != NULL);
					CU_ASSERT(root_tree->usage_counter = 1);
					ret = gitmod_root_tree_dispose_object(&object2);
					CU_ASSERT(ret);
					CU_ASSERT(object2 == NULL); //this object got cleared
					root_tree_deleted = ret;
				}
			}
			if (!root_tree_deleted) {
				// if it still exists, let's drop it
				gitmod_root_tree_dispose(&root_tree);
			}
			git_object_free(treeish);
		}
		git_repository_free(gitmod_info.repo);
	}
	git_libgit2_shutdown();
}

static void suitekim_testPullTwiceSameObjectMarkForDeletionDisposeOfThem()
{
	git_libgit2_init();
	int ret = git_repository_open(&gitmod_info.repo, ".");
	CU_ASSERT(!ret);
	if (!ret) {
		git_object * treeish;
		ret = git_revparse_single(&treeish, gitmod_info.repo, "81c15dc513f285e727e6e498d24474a885b7dc01"); // tree from v0.7
		CU_ASSERT(!ret);
		if (!ret) {
			gitmod_root_tree * root_tree = gitmod_root_tree_create((git_tree *) treeish, 0, 1);
			CU_ASSERT(root_tree != NULL);
			int root_tree_deleted = 0;
			if (root_tree) {
				CU_ASSERT(root_tree->usage_counter == 0);
				gitmod_object * object = gitmod_root_tree_get_object(root_tree, "/Makefile");
				CU_ASSERT(object != NULL);
				CU_ASSERT(root_tree->usage_counter = 2);
				gitmod_object * object2 = gitmod_root_tree_get_object(root_tree, "/Makefile");
				CU_ASSERT(object2 != NULL);
				CU_ASSERT(root_tree->usage_counter = 2);
				// we mark it to be disposed
				root_tree->marked_for_deletion=1;
				if (object != NULL) {
					ret = gitmod_root_tree_dispose_object(&object);
					CU_ASSERT(!ret);
					CU_ASSERT(object != NULL);
					CU_ASSERT(root_tree->usage_counter = 1);
					ret = gitmod_root_tree_dispose_object(&object2);
					CU_ASSERT(ret);
					CU_ASSERT(object2 == NULL); //this object got cleared
					root_tree_deleted = ret;
				}
			}
			if (!root_tree_deleted) {
				// if it still exists, let's drop it
				gitmod_root_tree_dispose(&root_tree);
			}
			git_object_free(treeish);
		}
		git_repository_free(gitmod_info.repo);
	}
	git_libgit2_shutdown();
}

CU_pSuite suitekim2_setup()
{
	CU_pSuite pSuite = CU_add_suite("Suitekim2", NULL, NULL);
	if (pSuite != NULL) {
		// did work
		if (!(CU_add_test(pSuite, "Suitekim2: pullTwoDifferentObjectsMarkForDeletionDisposeOfThem", suitekim_testPullTwoDifferentObjectsMarkForDeletionDisposeOfThem) &&
			CU_add_test(pSuite, "Suitekim2: pullTwiceSameObjectMarkForDeletionDisposeOfThem", suitekim_testPullTwiceSameObjectMarkForDeletionDisposeOfThem)
		)) {
			return NULL;
		}
	}
	return pSuite;
}

