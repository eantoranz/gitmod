/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#include "gitfs.h"
#include <string.h>
#include <stdio.h>
#include <stddef.h>


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

void gitfs_git_shutdown()
{
	if (gitfs_info.revision)
		git_object_free(gitfs_info.revision);
	// going out, for the time being
	git_libgit2_shutdown();
}


