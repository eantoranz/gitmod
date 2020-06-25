/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <CUnit/Basic.h>
#include "../include/gitfs.h"

int init_gitfs()
{
	return gitfs_init(".", "583510cd3ae56e");
}

void testRevisionInfo()
{
	printf("Revision time: %ld\n", gitfs_info.time);
	CU_ASSERT(gitfs_info.time == 1593046557);
}

void testGetRootTree()
{
	struct gitfs_object * root_tree;
        int res = gitfs_get_object(&root_tree, "/");
	CU_ASSERT(!res);
	if (!res) {
		CU_ASSERT(gitfs_get_object_type(root_tree) == GITFS_TREE);
		int num_items = gitfs_get_num_entries(root_tree);
		CU_ASSERT(num_items == 5);

		// let's check the names of each one of the entries
		struct gitfs_object * entry;
		for (int i=0; i < num_items; i++) {
			res = gitfs_get_tree_entry(&entry, root_tree, i);
			CU_ASSERT(!res);
			if (res == 0) {
				char * name = gitfs_get_name(entry);
				switch (i) {
					case 0:
						name = ".gitignore";
						break;
					case 1:
						name = "build.sh";
						break;
					case 2:
						name = "gitfs.c";
						break;
					case 3:
						name = "include";
						break;
					case 4:
						name = "tests";
						break;
					default:
						name = "***unknown item.... need to add more values***";
				}
				res = strcmp(name, gitfs_get_name(entry));
				CU_ASSERT(!res);
				if (res)
					printf("Item %d: Was expecting item to be named %s but got %s\n", i, name, gitfs_get_name(entry));
				gitfs_dispose(entry);
			}
		}
		// if we go over the board, we get not 0
		CU_ASSERT(gitfs_get_tree_entry(&entry, root_tree, 999) != 0);

		gitfs_dispose(root_tree);
	}
}

void testGetObjectByPathBlob()
{
	struct gitfs_object * object;
	int res = gitfs_get_object(&object, "/tests/test.c");
	CU_ASSERT(!res);
	if (!res) {
		CU_ASSERT(gitfs_get_object_type(object) == GITFS_BLOB);
		gitfs_dispose(object);
	}
}

void testGetObjectByPathTree()
{
	struct gitfs_object * object;
	int res = gitfs_get_object(&object, "tests");
	CU_ASSERT(!res);
	if (!res) {
		CU_ASSERT(gitfs_get_object_type(object) == GITFS_TREE);
		int tree_entries = gitfs_get_num_entries(object);
		printf("Entries en el tree: %d\n", tree_entries);
		CU_ASSERT(tree_entries == 1);
		gitfs_dispose(object);
	}
}

void testGetNonExistingObjectByPath()
{
	struct gitfs_object * object;
	int res = gitfs_get_object(&object, "blahblah");
	CU_ASSERT(res);
}

int shutdown_gitfs()
{
	gitfs_shutdown();
	return 0;
}

int main()
{
	CU_pSuite pSuite = NULL;

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* add a suite to the registry */
	pSuite = CU_add_suite("Suite_1", init_gitfs, shutdown_gitfs);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* add the tests to the suite */
	if (!(CU_add_test(pSuite, "test revision info", testRevisionInfo) &&
		CU_add_test(pSuite, "test of getRootTree", testGetRootTree) &&
		CU_add_test(pSuite, "test of getObjectByPathiBlob", testGetObjectByPathBlob) &&
		CU_add_test(pSuite, "test of getObjectByPathTree", testGetObjectByPathTree) &&
		CU_add_test(pSuite, "test of getNonExisingObjectByPath", testGetNonExistingObjectByPath)))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
