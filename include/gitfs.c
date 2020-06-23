/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include "gitfs.h"
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <git2.h>


int gitfs_git_init(const char * repo_path, const char * treeish)
{
	int ret;

	// save the treeish
	gitfs_info.treeish = treeish;

	git_libgit2_init();
	ret = git_repository_open(&gitfs_info.repo, repo_path);
	if (ret) {
		// there was an error opening the repository
		fprintf(stderr, "There was an error opening the git repo at %s\n", repo_path);
		goto end;
	}

	printf("Successfully opened repo at %s\n", git_repository_commondir(gitfs_info.repo));
	ret = git_revparse_single(&gitfs_info.revision, gitfs_info.repo, treeish);
	if (ret) {
		fprintf(stderr, "There was error parsing the threeish %s on the repo\n", treeish);
		goto end;
	}

	// TODO how can I make sure the object is a git_commit?
	printf("Successfully parsed treeish %s\n", treeish);
	ret =  git_commit_tree(&gitfs_info.root_tree, (git_commit *) gitfs_info.revision);
	if (ret) {
		fprintf(stderr, "Could not find tree object for the revision\n");
		goto end;
	}

	printf("Using tree %s as the root of the mount point\n", git_oid_tostr_s(git_tree_id(gitfs_info.root_tree)));
end:
	return ret;
}

static int gitfs_get_object(git_object ** object, const char *path)
{
	int ret = 0;
	char object_path [strlen(gitfs_info.treeish) + 1 + strlen(path)];
	strcpy(object_path, gitfs_info.treeish);
	strcat(object_path, ":");
	strcat(object_path, path + 1);

	ret = git_revparse_single(object, gitfs_info.repo, object_path);
	if (ret) {
		fprintf(stderr, "Could not find the object for the path %s\n", path);
	}
	return ret;
}

int gitfs_get_num_items(const char * path)
{
	printf("Getting the number of items in path %s\n", path);
	git_object * tree;
	int ret = 0;

	ret = gitfs_get_object(&tree, path);
	if (ret) {
		return -ENOENT;
	}
	// found the object
	ret = git_tree_entrycount((git_tree *) tree);

	// going out
	git_object_free(tree);

	return ret;
}

char * gitfs_get_entry_name(const char * tree_path, int index)
{
	char * name = NULL;
	git_object * tree = NULL;
	printf("Getting name of entry %d in tree with path %s\n", index, tree_path);
	int ret = gitfs_get_object(&tree, tree_path);

	if (ret) {
		// could not get tree
		goto end;
	}
	// found the tree
	const git_tree_entry * entry = git_tree_entry_byindex((const git_tree *) tree, index);
	if (!entry) {
		goto end;
	}
	name = strdup(git_tree_entry_name(entry));
	printf("Entry name: %s\n", name);
end:
	if (tree) {
		git_object_free(tree);
	}
	return name;
}

void gitfs_git_shutdown()
{
	if (gitfs_info.revision)
		git_object_free(gitfs_info.revision);
	// going out, for the time being
	git_libgit2_shutdown();
}


