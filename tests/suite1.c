/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 * 
 * Suite 1
 */

#include "object.h"
#include "gitmod.h"
#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>

static int suite1_init_gitmod()
{
	return gitmod_init(".", "583510cd3ae56e");
}

static void suite1_testRevisionInfo()
{
	fprintf(stderr, "Revision time: %ld\n", gitmod_info.root_tree->time);
	CU_ASSERT(gitmod_info.root_tree->time == 1593046557);
	CU_ASSERT(gitmod_info.treeish_type == GIT_OBJ_COMMIT);
}

static void suite1_testGetRootTree()
{
	gitmod_object * root_tree = gitmod_get_object("/", 1);
	CU_ASSERT(root_tree != NULL);
	if (root_tree) {
		CU_ASSERT(gitmod_object_get_mode(root_tree) == 0555);
		CU_ASSERT(gitmod_object_get_type(root_tree) == GITMOD_OBJECT_TREE);
		int num_items = gitmod_object_get_num_entries(root_tree);
		CU_ASSERT(num_items == 5);

		// let's check the names of each one of the entries
		gitmod_object * entry;
		for (int i=0; i < num_items; i++) {
			entry = gitmod_get_tree_entry(root_tree, i, 1);
			CU_ASSERT(entry != NULL);
			if (entry) {
				char * name = gitmod_object_get_name(entry);
				int expected_items;
				int expected_size;
				int expected_mode;
				enum gitmod_object_type expected_type;
				switch (i) {
					case 0:
						name = ".gitignore";
						expected_items = 1;
						expected_type = GITMOD_OBJECT_BLOB;
						expected_size = 17;
						expected_mode = 0444;
						break;
					case 1:
						name = "build.sh";
						expected_items = 1;
						expected_type = GITMOD_OBJECT_BLOB;
						expected_size = 274;
						expected_mode = 0555;
						break;
					case 2:
						name = "gitfs.c";
						expected_items = 1;
						expected_type = GITMOD_OBJECT_BLOB;
						expected_size = 4672;
						expected_mode = 0444;
						break;
					case 3:
						name = "include";
						expected_items = 2;
						expected_type = GITMOD_OBJECT_TREE;
						expected_size = 2;
						expected_mode = 0;
						break;
					case 4:
						name = "tests";
						expected_items = 1;
						expected_type = GITMOD_OBJECT_TREE;
						expected_size = 1;
						expected_mode = 0;
						break;
					default:
						name = "***unknown item.... need to add more values***";
						expected_type = GITMOD_OBJECT_UNKNOWN;
						expected_items = -ENOENT;
						expected_size = -765;
						expected_mode = 0;
				}
				int res = strcmp(name, gitmod_object_get_name(entry));
				CU_ASSERT(!res);
				if (res)
					fprintf(stderr, "Item %d: Was expecting item to be named %s but got %s\n", i, name, gitmod_object_get_name(entry));
				CU_ASSERT(gitmod_object_get_num_entries(entry) == expected_items);
				CU_ASSERT(gitmod_object_get_type(entry) == expected_type);
				CU_ASSERT(gitmod_object_get_size(entry) == expected_size);
				CU_ASSERT(gitmod_object_get_mode(entry) == expected_mode);
				if (gitmod_object_get_mode(entry) != expected_mode) {
					fprintf(stderr, "Mode for %s was %o but %o was expected\n", name, gitmod_object_get_mode(entry), expected_mode);
				}
				gitmod_object_dispose(&entry);
			}
		}
		// if we go over the board, we get NULL
		CU_ASSERT(gitmod_get_tree_entry(root_tree, 999, 0) == NULL);

		gitmod_object_dispose(&root_tree);
	}
}

static void suite1_testGetObjectByPathBlob()
{
	gitmod_object * object = gitmod_get_object("/tests/test.c", 1);
	CU_ASSERT(object != NULL);
	if (object) {
		CU_ASSERT(gitmod_object_get_type(object) == GITMOD_OBJECT_BLOB);
		CU_ASSERT(gitmod_object_get_num_entries(object) == 1);
		long size = gitmod_object_get_size(object);
		CU_ASSERT(size == 2078);
		const char * content = gitmod_object_get_content(object);
		CU_ASSERT(content != NULL);
		if (content) {
			char * dest = malloc(10);
			strncpy(dest, content, 9);
			CU_ASSERT(strcmp(dest, "/*\n * Cop") == 0);
		}
		CU_ASSERT(gitmod_object_get_mode(object) == 0444);
		gitmod_object_dispose(&object);
	}
}

static void suite1_testGetExecObjectByPathBlob()
{
	gitmod_object * object = gitmod_get_object("/build.sh", 1);
	CU_ASSERT(object != NULL);
	if (object) {
		CU_ASSERT(gitmod_object_get_type(object) == GITMOD_OBJECT_BLOB);
		CU_ASSERT(gitmod_object_get_num_entries(object) == 1);
		long size = gitmod_object_get_size(object);
		CU_ASSERT(size == 274);
		const char * content = gitmod_object_get_content(object);
		CU_ASSERT(content != NULL);
		if (content) {
			char * dest = malloc(10);
			strncpy(dest, content, 9);
			CU_ASSERT(strcmp(dest, "#!/bin/ba") == 0);
		}
		CU_ASSERT(gitmod_object_get_mode(object) == 0555);
		gitmod_object_dispose(&object);
	}
}

static void suite1_testGetObjectByPathTree()
{
	gitmod_object * object = gitmod_get_object("tests", 1);
	CU_ASSERT(object != NULL);
	if (object) {
		CU_ASSERT(gitmod_object_get_type(object) == GITMOD_OBJECT_TREE);
		int tree_entries = gitmod_object_get_num_entries(object);
		CU_ASSERT(tree_entries == 1);
		CU_ASSERT(gitmod_object_get_content(object) == NULL);
		CU_ASSERT(gitmod_object_get_mode(object) == 0);
		gitmod_object_dispose(&object);
	}
}

static void suite1_testGetObjectByPathTreeWithoutMode()
{
	gitmod_object * object = gitmod_get_object("tests", 0);
	CU_ASSERT(object != NULL);
	if (object) {
		CU_ASSERT(gitmod_object_get_type(object) == GITMOD_OBJECT_TREE);
		int tree_entries = gitmod_object_get_num_entries(object);
		CU_ASSERT(tree_entries == 1);
		CU_ASSERT(gitmod_object_get_content(object) == NULL);
		CU_ASSERT(gitmod_object_get_mode(object) == 0);
		gitmod_object_dispose(&object);
	}
}

static void suite1_testGetNonExistingObjectByPath()
{
	gitmod_object * object = gitmod_get_object("blahblah", 1);
	CU_ASSERT(object == NULL);
}

static int suite1_shutdown_gitmod()
{
	gitmod_shutdown();
	return 0;
}

CU_pSuite suite1_setup()
{
	CU_pSuite pSuite = CU_add_suite("Suite1", suite1_init_gitmod, suite1_shutdown_gitmod);
	if (pSuite != NULL) {
		// did work
		if (!(CU_add_test(pSuite, "Suite1: test revision info", suite1_testRevisionInfo) &&
		CU_add_test(pSuite, "Suite1: getRootTree", suite1_testGetRootTree) &&
		CU_add_test(pSuite, "Suite1: getObjectByPathBlob", suite1_testGetObjectByPathBlob) &&
		CU_add_test(pSuite, "Suite1: getExecObjectByPathBlob", suite1_testGetExecObjectByPathBlob) &&
		CU_add_test(pSuite, "Suite1: getObjectByPathTree", suite1_testGetObjectByPathTree) &&
		CU_add_test(pSuite, "Suite1: suite1_testGetObjectByPathTreeWithoutMode", suite1_testGetObjectByPathTreeWithoutMode) &&
		CU_add_test(pSuite, "Suite1: getNonExisingObjectByPath", suite1_testGetNonExistingObjectByPath))) {
			return NULL;
		}
	}
	return pSuite;
}

