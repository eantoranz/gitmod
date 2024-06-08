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

static gitmod_info *gm_info;

static int suitekim2_init()
{
	gitmod_init();
	gm_info = calloc(1, sizeof(gitmod_info));
	return 0;
}

static int suitekim2_shutdown()
{
	gitmod_shutdown();
	free(gm_info);
	return 0;
}

static void suitekim2_testPullTwoDifferentObjectsMarkForDeletionDisposeOfThem()
{
	int ret = git_repository_open(&gm_info->repo, ".");
	CU_ASSERT(!ret);
	if (!ret) {
		git_object *treeish;
		ret = git_revparse_single(&treeish, gm_info->repo, "81c15dc513f285e727e6e498d24474a885b7dc01");	// tree from v0.7
		CU_ASSERT(!ret);
		if (!ret) {
			gitmod_root_tree *root_tree = gitmod_root_tree_create((git_tree *) treeish, 0, 1);
			CU_ASSERT(root_tree != NULL);
			int root_tree_deleted = 0;
			if (root_tree) {
				CU_ASSERT(root_tree->usage_counter == 0);
				gitmod_object *object = gitmod_root_tree_get_object(gm_info, root_tree, "/Makefile");
				CU_ASSERT(object != NULL);
				CU_ASSERT(root_tree->usage_counter = 2);
				gitmod_object *object2 =
				    gitmod_root_tree_get_object(gm_info, root_tree, "/src/gitmod.c");
				CU_ASSERT(object2 != NULL);
				CU_ASSERT(root_tree->usage_counter = 2);
				// we mark it to be disposed
				root_tree->marked_for_deletion = 1;
				if (object != NULL) {
					ret = gitmod_root_tree_dispose_object(&object);
					CU_ASSERT(!ret);
					CU_ASSERT(object != NULL);
					CU_ASSERT(root_tree->usage_counter = 1);
					ret = gitmod_root_tree_dispose_object(&object2);
					CU_ASSERT(ret);
					CU_ASSERT(object2 == NULL);	//this object got cleared
					root_tree_deleted = ret;
				}
			}
			if (!root_tree_deleted) {
				// if it still exists, let's drop it
				gitmod_root_tree_dispose(&root_tree);
			}
			git_object_free(treeish);
		}
		git_repository_free(gm_info->repo);
	}
}

static void suitekim2_testPullTwiceSameObjectMarkForDeletionDisposeOfThem()
{
	int ret = git_repository_open(&gm_info->repo, ".");
	CU_ASSERT(!ret);
	if (!ret) {
		git_object *treeish;
		ret = git_revparse_single(&treeish, gm_info->repo, "81c15dc513f285e727e6e498d24474a885b7dc01");	// tree from v0.7
		CU_ASSERT(!ret);
		if (!ret) {
			gitmod_root_tree *root_tree = gitmod_root_tree_create((git_tree *) treeish, 0, 1);
			CU_ASSERT(root_tree != NULL);
			int root_tree_deleted = 0;
			if (root_tree) {
				CU_ASSERT(root_tree->usage_counter == 0);
				gitmod_object *object = gitmod_root_tree_get_object(gm_info, root_tree, "/Makefile");
				CU_ASSERT(object != NULL);
				CU_ASSERT(root_tree->usage_counter = 2);
				gitmod_object *object2 = gitmod_root_tree_get_object(gm_info, root_tree, "/Makefile");
				CU_ASSERT(object2 != NULL);
				CU_ASSERT(root_tree->usage_counter = 2);
				// we mark it to be disposed
				root_tree->marked_for_deletion = 1;
				if (object != NULL) {
					ret = gitmod_root_tree_dispose_object(&object);
					CU_ASSERT(!ret);
					CU_ASSERT(object != NULL);
					CU_ASSERT(root_tree->usage_counter = 1);
					ret = gitmod_root_tree_dispose_object(&object2);
					CU_ASSERT(ret);
					CU_ASSERT(object2 == NULL);	//this object got cleared
					root_tree_deleted = ret;
				}
			}
			if (!root_tree_deleted) {
				// if it still exists, let's drop it
				gitmod_root_tree_dispose(&root_tree);
			}
			git_object_free(treeish);
		}
		git_repository_free(gm_info->repo);
	}
}

static void suitekim2_treeMovesNoObjectInUse()
{
	int ret = git_repository_open(&gm_info->repo, ".");
	CU_ASSERT(!ret);
	if (!ret) {
		git_object *treeish;
		ret = git_revparse_single(&treeish, gm_info->repo, "v0.7^{tree}");
		CU_ASSERT(!ret);
		if (!ret) {
			gitmod_root_tree *root_tree = gitmod_root_tree_create((git_tree *) treeish, 0, 1);
			CU_ASSERT(root_tree != NULL);
			if (root_tree) {
				CU_ASSERT(root_tree->usage_counter == 0);
				gm_info->root_tree = root_tree;
				// will askto change the tree for a new one
				ret = git_revparse_single(&treeish, gm_info->repo, "v0.6^{tree}");
				CU_ASSERT(!ret);
				if (!ret) {
					gitmod_root_tree *root_tree2 =
					    gitmod_root_tree_create((git_tree *) treeish, 0, 1);
					CU_ASSERT(root_tree2 != NULL);
					gitmod_lock(gm_info->lock);
					ret = gitmod_root_tree_changed(gm_info, root_tree2);
					CU_ASSERT(ret != 0);	// original root tree was deleted
					CU_ASSERT(gm_info->root_tree == root_tree2);
					if (!ret) {
						// failed test, root tree was not deleted
						gitmod_root_tree_dispose(&root_tree);
					}
					gitmod_root_tree_dispose(&root_tree2);
				}
			}
		}
	}
}

static void suitekim2_treeMoves1ObjectInUse()
{
	int ret = git_repository_open(&gm_info->repo, ".");
	CU_ASSERT(!ret);
	if (!ret) {
		git_object *treeish;
		ret = git_revparse_single(&treeish, gm_info->repo, "v0.7^{tree}");
		CU_ASSERT(!ret);
		if (!ret) {
			gitmod_root_tree *root_tree = gitmod_root_tree_create((git_tree *) treeish, 0, 1);
			CU_ASSERT(root_tree != NULL);
			if (root_tree) {
				CU_ASSERT(root_tree->usage_counter == 0);
				gm_info->root_tree = root_tree;

				// pull an object from tree
				gitmod_object *object = gitmod_root_tree_get_object(gm_info, root_tree, "/Makefile");
				CU_ASSERT(object != NULL);
				CU_ASSERT(root_tree->usage_counter == 1);
				// will askto change the tree for a new one
				ret = git_revparse_single(&treeish, gm_info->repo, "v0.6^{tree}");
				CU_ASSERT(!ret);
				if (!ret) {
					gitmod_root_tree *root_tree2 =
					    gitmod_root_tree_create((git_tree *) treeish, 0, 1);
					CU_ASSERT(root_tree2 != NULL);
					gitmod_lock(gm_info->lock);
					ret = gitmod_root_tree_changed(gm_info, root_tree2);
					CU_ASSERT(!ret);	// original root tree _not_ was deleted because the object is still in use
					CU_ASSERT(gm_info->root_tree == root_tree2);
					// when we drop the object, original tree should be killed
					ret = gitmod_root_tree_dispose_object(&object);
					CU_ASSERT(ret);
					if (!ret) {
						// failed test, root tree was _not_ deleted
						gitmod_root_tree_dispose(&root_tree);
					}
					gitmod_root_tree_dispose(&root_tree2);
				}
			}
		}
	}
}

static void suitekim2_treeMoves1ObjectInUseTwice()
{
	int ret = git_repository_open(&gm_info->repo, ".");
	CU_ASSERT(!ret);
	if (!ret) {
		git_object *treeish;
		ret = git_revparse_single(&treeish, gm_info->repo, "v0.7^{tree}");
		CU_ASSERT(!ret);
		if (!ret) {
			gitmod_root_tree *root_tree = gitmod_root_tree_create((git_tree *) treeish, 0, 1);
			CU_ASSERT(root_tree != NULL);
			if (root_tree) {
				CU_ASSERT(root_tree->usage_counter == 0);
				gm_info->root_tree = root_tree;

				// pull an object from tree
				gitmod_object *object = gitmod_root_tree_get_object(gm_info, root_tree, "/Makefile");
				CU_ASSERT(object != NULL);
				CU_ASSERT(object->root_tree == root_tree);
				CU_ASSERT(root_tree->usage_counter == 1);
				gitmod_object *object2 = gitmod_root_tree_get_object(gm_info, root_tree, "/Makefile");
				CU_ASSERT(object2 != NULL);
				CU_ASSERT(object2->root_tree == root_tree);
				CU_ASSERT(root_tree->usage_counter == 2);
				CU_ASSERT(object == object2);
				// will askto change the tree for a new one
				ret = git_revparse_single(&treeish, gm_info->repo, "v0.6^{tree}");
				CU_ASSERT(!ret);
				if (!ret) {
					gitmod_root_tree *root_tree2 =
					    gitmod_root_tree_create((git_tree *) treeish, 0, 1);
					CU_ASSERT(root_tree2 != NULL);
					gitmod_lock(gm_info->lock);
					CU_ASSERT(root_tree->usage_counter == 2);
					ret = gitmod_root_tree_changed(gm_info, root_tree2);
					CU_ASSERT(!ret);	// original root tree _not_ was deleted because the object is still in use
					CU_ASSERT(gm_info->root_tree == root_tree2);
					CU_ASSERT(root_tree->usage_counter == 2);
					// when we drop the object, original tree should be killed
					ret = gitmod_root_tree_dispose_object(&object);
					CU_ASSERT(!ret);
					CU_ASSERT(root_tree->usage_counter == 1);
					ret = gitmod_root_tree_dispose_object(&object2);
					CU_ASSERT(ret);
					if (!ret) {
						// failed test, root tree was _not_ deleted
						gitmod_root_tree_dispose(&root_tree);
					}
					gitmod_root_tree_dispose(&root_tree2);
				}
			}
		}
	}
}

static void suitekim2_treeMoves2ObjectsInUse()
{
	int ret = git_repository_open(&gm_info->repo, ".");
	CU_ASSERT(!ret);
	if (!ret) {
		git_object *treeish;
		ret = git_revparse_single(&treeish, gm_info->repo, "v0.7^{tree}");
		CU_ASSERT(!ret);
		if (!ret) {
			gitmod_root_tree *root_tree = gitmod_root_tree_create((git_tree *) treeish, 0, 1);
			CU_ASSERT(root_tree != NULL);
			if (root_tree) {
				CU_ASSERT(root_tree->usage_counter == 0);
				gm_info->root_tree = root_tree;

				// pull an object from tree
				gitmod_object *object = gitmod_root_tree_get_object(gm_info, root_tree, "/Makefile");
				CU_ASSERT(object != NULL);
				CU_ASSERT(object->root_tree == root_tree);
				CU_ASSERT(root_tree->usage_counter == 1);
				gitmod_object *object2 = gitmod_root_tree_get_object(gm_info, root_tree, "/readme.txt");
				CU_ASSERT(object2 != NULL);
				CU_ASSERT(object2->root_tree == root_tree);
				CU_ASSERT(root_tree->usage_counter == 2);
				CU_ASSERT(object != object2);
				// will askto change the tree for a new one
				ret = git_revparse_single(&treeish, gm_info->repo, "v0.6^{tree}");
				CU_ASSERT(!ret);
				if (!ret) {
					gitmod_root_tree *root_tree2 =
					    gitmod_root_tree_create((git_tree *) treeish, 0, 1);
					CU_ASSERT(root_tree2 != NULL);
					gitmod_lock(gm_info->lock);
					CU_ASSERT(root_tree->usage_counter == 2);
					ret = gitmod_root_tree_changed(gm_info, root_tree2);
					CU_ASSERT(!ret);	// original root tree _not_ was deleted because the object is still in use
					CU_ASSERT(gm_info->root_tree == root_tree2);
					CU_ASSERT(root_tree->usage_counter == 2);
					// when we drop the object, original tree should be killed
					ret = gitmod_root_tree_dispose_object(&object);
					CU_ASSERT(!ret);
					CU_ASSERT(root_tree->usage_counter == 1);
					ret = gitmod_root_tree_dispose_object(&object2);
					CU_ASSERT(ret);
					if (!ret) {
						// failed test, root tree was _not_ deleted
						gitmod_root_tree_dispose(&root_tree);
					}
					gitmod_root_tree_dispose(&root_tree2);
				}
			}
		}
	}
}

CU_pSuite suitekim2_setup()
{
	CU_pSuite pSuite = CU_add_suite("Suitekim2", suitekim2_init, suitekim2_shutdown);
	if (pSuite != NULL) {
		// did work
		if (!
		    (CU_add_test
		     (pSuite, "Suitekim2: pullTwoDifferentObjectsMarkForDeletionDisposeOfThem",
		      suitekim2_testPullTwoDifferentObjectsMarkForDeletionDisposeOfThem)
		     && CU_add_test(pSuite, "Suitekim2: pullTwiceSameObjectMarkForDeletionDisposeOfThem",
				    suitekim2_testPullTwiceSameObjectMarkForDeletionDisposeOfThem)
		     && CU_add_test(pSuite, "Suitekim2, treeMovesNoObjectInUse", suitekim2_treeMovesNoObjectInUse)
		     && CU_add_test(pSuite, "Suitekim2, treeMoves1ObjectInUse", suitekim2_treeMoves1ObjectInUse)
		     && CU_add_test(pSuite, "Suitekim2, treeMoves1ObjectInUseTwice",
				    suitekim2_treeMoves1ObjectInUseTwice)
		     && CU_add_test(pSuite, "Suitekim2, treeMoves2ObjectsInUse", suitekim2_treeMoves2ObjectsInUse))) {
			return NULL;
		}
	}
	return pSuite;
}
