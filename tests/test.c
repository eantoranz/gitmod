/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include <CUnit/Basic.h>
#include "../include/gitfstrack.h"

int init_gitfs()
{
	return gitfs_init(".", "583510cd3ae56e");
}

void testRevisionInfo()
{
	fprintf(stderr, "Revision time: %ld\n", gitfs_info.time);
	CU_ASSERT(gitfs_info.time == 1593046557);
}

void testGetRootTree()
{
	struct gitfs_object * root_tree = gitfs_get_object("/", 1);
	CU_ASSERT(root_tree != NULL);
	if (root_tree) {
		CU_ASSERT(gitfs_get_mode(root_tree) == 0555);
		CU_ASSERT(gitfs_get_type(root_tree) == GITFS_TREE);
		int num_items = gitfs_get_num_entries(root_tree);
		CU_ASSERT(num_items == 5);

		// let's check the names of each one of the entries
		struct gitfs_object * entry;
		for (int i=0; i < num_items; i++) {
			entry = gitfs_get_tree_entry(root_tree, i, 1);
			CU_ASSERT(entry != NULL);
			if (entry) {
				char * name = gitfs_get_name(entry);
				int expected_items;
				int expected_size;
				int expected_mode;
				enum gitfs_object_type expected_type;
				switch (i) {
					case 0:
						name = ".gitignore";
						expected_items = 1;
						expected_type = GITFS_BLOB;
						expected_size = 17;
						expected_mode = 0444;
						break;
					case 1:
						name = "build.sh";
						expected_items = 1;
						expected_type = GITFS_BLOB;
						expected_size = 274;
						expected_mode = 0555;
						break;
					case 2:
						name = "gitfs.c";
						expected_items = 1;
						expected_type = GITFS_BLOB;
						expected_size = 4672;
						expected_mode = 0444;
						break;
					case 3:
						name = "include";
						expected_items = 2;
						expected_type = GITFS_TREE;
						expected_size = 2;
						expected_mode = 0;
						break;
					case 4:
						name = "tests";
						expected_items = 1;
						expected_type = GITFS_TREE;
						expected_size = 1;
						expected_mode = 0;
						break;
					default:
						name = "***unknown item.... need to add more values***";
						expected_type = GITFS_UNKNOWN;
						expected_items = -ENOENT;
						expected_size = -765;
						expected_mode = 0;
				}
				int res = strcmp(name, gitfs_get_name(entry));
				CU_ASSERT(!res);
				if (res)
					fprintf(stderr, "Item %d: Was expecting item to be named %s but got %s\n", i, name, gitfs_get_name(entry));
				CU_ASSERT(gitfs_get_num_entries(entry) == expected_items);
				CU_ASSERT(gitfs_get_type(entry) == expected_type);
				CU_ASSERT(gitfs_get_size(entry) == expected_size);
				CU_ASSERT(gitfs_get_mode(entry) == expected_mode);
				if (gitfs_get_mode(entry) != expected_mode) {
					fprintf(stderr, "Mode for %s was %o but %o was expected\n", name, gitfs_get_mode(entry), expected_mode);
				}
				gitfs_dispose(entry);
			}
		}
		// if we go over the board, we get NULL
		CU_ASSERT(gitfs_get_tree_entry(root_tree, 999, 0) == NULL);

		gitfs_dispose(root_tree);
	}
}

void testGetObjectByPathBlob()
{
	struct gitfs_object * object = gitfs_get_object("/tests/test.c", 1);
	CU_ASSERT(object != NULL);
	if (object) {
		CU_ASSERT(gitfs_get_type(object) == GITFS_BLOB);
		CU_ASSERT(gitfs_get_num_entries(object) == 1);
		long size = gitfs_get_size(object);
		CU_ASSERT(size == 2078);
		const char * content = gitfs_get_content(object);
		CU_ASSERT(content != NULL);
		if (content) {
			char * dest = malloc(10);
			strncpy(dest, content, 9);
			CU_ASSERT(strcmp(dest, "/*\n * Cop") == 0);
		}
		CU_ASSERT(gitfs_get_mode(object) == 0444);
		gitfs_dispose(object);
	}
}

void testGetExecObjectByPathBlob()
{
	struct gitfs_object * object = gitfs_get_object("/build.sh", 1);
	CU_ASSERT(object != NULL);
	if (object) {
		CU_ASSERT(gitfs_get_type(object) == GITFS_BLOB);
		CU_ASSERT(gitfs_get_num_entries(object) == 1);
		long size = gitfs_get_size(object);
		CU_ASSERT(size == 274);
		const char * content = gitfs_get_content(object);
		CU_ASSERT(content != NULL);
		if (content) {
			char * dest = malloc(10);
			strncpy(dest, content, 9);
			CU_ASSERT(strcmp(dest, "#!/bin/ba") == 0);
		}
		CU_ASSERT(gitfs_get_mode(object) == 0555);
		gitfs_dispose(object);
	}
}

void testGetObjectByPathTree()
{
	struct gitfs_object * object = gitfs_get_object("tests", 1);
	CU_ASSERT(object != NULL);
	if (object) {
		CU_ASSERT(gitfs_get_type(object) == GITFS_TREE);
		int tree_entries = gitfs_get_num_entries(object);
		CU_ASSERT(tree_entries == 1);
		CU_ASSERT(gitfs_get_content(object) == NULL);
		CU_ASSERT(gitfs_get_mode(object) == 0);
		gitfs_dispose(object);
	}
}

void testGetObjectByPathTreeWithoutMode()
{
	struct gitfs_object * object = gitfs_get_object("tests", 0);
	CU_ASSERT(object != NULL);
	if (object) {
		CU_ASSERT(gitfs_get_type(object) == GITFS_TREE);
		int tree_entries = gitfs_get_num_entries(object);
		CU_ASSERT(tree_entries == 1);
		CU_ASSERT(gitfs_get_content(object) == NULL);
		CU_ASSERT(gitfs_get_mode(object) == 0);
		gitfs_dispose(object);
	}
}

void testGetNonExistingObjectByPath()
{
	struct gitfs_object * object = gitfs_get_object("blahblah", 1);
	CU_ASSERT(!object);
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
		CU_add_test(pSuite, "test of getObjectByPathBlob", testGetObjectByPathBlob) &&
		CU_add_test(pSuite, "test of getExecObjectByPathBlob", testGetExecObjectByPathBlob) &&
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
