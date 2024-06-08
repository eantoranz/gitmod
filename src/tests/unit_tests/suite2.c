/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 * 
 * Suite 2
 *  Make sure of the kinds of objects we can use as treeish
 */

#include "gitmod.h"
#include <CUnit/Basic.h>

// TODO add tests for signed tags

static gitmod_info *gm_info;
static char *REPO_PATH = "tests/test_repo";

static int suite2_init()
{
	gitmod_init();
	return 0;
}

static int suite2_shutdown()
{
	gitmod_shutdown();
	return 0;
}

static void suite2_treeish_is_tree()
{
	gm_info = gitmod_start(REPO_PATH, "65517b96a487fbf59775aaefae3f8faff634ae79", 0, 100);	// tree of intermediate version
	CU_ASSERT(gm_info != NULL);
	if (gm_info) {
		CU_ASSERT(gm_info->treeish_type == GIT_OBJ_TREE);
		// should check that we can list stuff from here
		gitmod_object *tree = gitmod_get_object(gm_info, "/");
		CU_ASSERT(tree != NULL);
		if (tree) {
			CU_ASSERT(gitmod_object_get_num_entries(tree) == 4);
			gitmod_dispose_object(&tree);
		}
		// In case this worked, so that we can run other tests
		gitmod_stop(&gm_info);
	}
}

static void suite2_treeish_is_blob()
{
	gm_info = gitmod_start("tests/test_repo", "b9246be10bb6ea5938cf0a20cca4a1762dedce2e", 0, 100);	// readme of intermediate version
	CU_ASSERT(gm_info == NULL);
	if (gm_info) {
		// In case this worked, so that we can run other tests
		gitmod_stop(&gm_info);
	}
}

static void suite2_treeish_is_tag()
{
	gm_info = gitmod_start("tests/test_repo", "intermediate", 0, 100);
	CU_ASSERT(gm_info != NULL);
	if (gm_info) {
		CU_ASSERT(gm_info->treeish_type == GIT_OBJ_COMMIT);
		// so that we can run other tests
		gitmod_stop(&gm_info);
	}
}

CU_pSuite suite2_setup()
{
	git_libgit2_init();
	CU_pSuite pSuite = CU_add_suite("Suite2", suite2_init, suite2_shutdown);
	if (pSuite != NULL) {
		// did work
		if (!(CU_add_test(pSuite, "Suite2: treeish_is_object", suite2_treeish_is_tree) &&
		      CU_add_test(pSuite, "Suite2: treeish_is_tag", suite2_treeish_is_blob) &&
		      CU_add_test(pSuite, "Suite2: treeish_is_tag", suite2_treeish_is_tag))) {
			return NULL;
		}
	}
	return pSuite;
}
