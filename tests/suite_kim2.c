/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 * 
 * Suite keep in memory
 */

#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
//#include "object.h"
#include "gitmod.h"
//#include "cache.h"
//#include "root_tree.h"

static void suitekim_testUseObjectAfterRootTreeIsKilled()
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
			if (root_tree) {
				gitmod_object * object = gitmod_root_tree_get_object(root_tree, "/Makefile");
				CU_ASSERT(object != NULL);
				if (object != NULL) {
					CU_ASSERT(object->usage == 1);
					CU_ASSERT(!strcmp("/Makefile", object->path));
					gitmod_root_tree_dispose(&root_tree);
					CU_ASSERT(!strcmp("/Makefile", object->path));
					gitmod_object_dispose(&object); // now we can dispose of it and it should work
					CU_ASSERT(object == NULL);
				}
			}
			if (root_tree) {
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
		if (!(CU_add_test(pSuite, "Suitekim2: testUseObjectAfterRootTreeIsKilled", suitekim_testUseObjectAfterRootTreeIsKilled))
		) {
			return NULL;
		}
	}
	return pSuite;
}

