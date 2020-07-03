/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 * 
 * Suite 2
 *  Make sure we can NOT work on a treeish that is _not_ a revision
 */

#include "gitmod.h"
#include <CUnit/Basic.h>

static void suite2_treeish_is_tree()
{
	int ret = gitmod_init(".", "99a4ae6337c961b967e9f91c328c85c3f01e7aaf"); // tree of v0.4
	CU_ASSERT(ret == 0);
	if (!ret) {
		// should check that we can list stuff from here
		gitmod_object * tree = gitmod_get_object("/", 0);
		CU_ASSERT(tree != NULL);
		if (tree) {
			CU_ASSERT(gitmod_get_num_entries(tree) == 6);
			gitmod_dispose(tree);
		}
		// In case this worked, so that we can run other tests
		gitmod_shutdown();
	}
}

static void suite2_treeish_is_blob()
{
	int ret = gitmod_init(".", "9b04567a7703417459bceb4703825dfd7a81725c"); // .gitignore of v0.4
	CU_ASSERT(ret != 0);
	if (!ret) {
		// In case this worked, so that we can run other tests
		gitmod_shutdown();
	}
}

static void suite2_treeish_is_tag()
{
	int ret = gitmod_init(".", "v0.4");
	CU_ASSERT(ret == 0); // tree of v0.4
	if (!ret) {
		// so that we can run other tests
		gitmod_shutdown();
	}
}

CU_pSuite suite2_setup()
{
	CU_pSuite pSuite = CU_add_suite("Suite2", NULL, NULL);
	if (pSuite != NULL) {
		// did work
		if (!(CU_add_test(pSuite, "Suite2: treeish_is_object", suite2_treeish_is_tree) &&
			CU_add_test(pSuite, "Suite2: treeish_is_tag", suite2_treeish_is_blob) &&
			CU_add_test(pSuite, "Suite2: treeish_is_tag", suite2_treeish_is_tag)
		)) {
			return NULL;
		}
	}
	return pSuite;
}

