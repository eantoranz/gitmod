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

struct gitfs_object {
	git_tree * tree;
	git_tree_entry * tree_entry;
	int git_tree_entry_dispose; // do not dispose manually
	char * name; // local name, _not_ fullpath
} gitfs_object;

int gitfs_init(const char * repo_path, const char * treeish)
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

void gitfs_dispose(struct gitfs_object * object)
{
	if (object->tree && object->tree != gitfs_info.root_tree)
		git_tree_free(object->tree);
	if (object->tree_entry && object->git_tree_entry_dispose)
		git_tree_entry_free(object->tree_entry);
	if (object->name)
		free(object->name);

	// finally
	free(object);
}


int gitfs_get_object(struct gitfs_object ** object, const char *path)
{
	int ret = 0;
	if (!(strlen(path) && strcmp(path, "/"))) {
		// root tree
		object[0] = calloc(1, sizeof(gitfs_object));
		object[0]->tree = gitfs_info.root_tree;
		return 0;
	}
	char object_path [strlen(gitfs_info.treeish) + 1 + strlen(path)];
	strcpy(object_path, gitfs_info.treeish);
	strcat(object_path, ":");
	strcat(object_path, path + 1);

	struct gitfs_object gitfs_object;
	ret = git_tree_entry_bypath(&gitfs_object.tree_entry, gitfs_info.root_tree, object_path);
	if (ret) {
		fprintf(stderr, "Could not find the object for the path %s\n", path);
		goto end;
	}
	object[0] = calloc(1, sizeof(gitfs_object));
	object[0]->tree_entry = gitfs_object.tree_entry;
end:
	return ret;
}

int gitfs_get_num_entries(struct gitfs_object * tree)
{
	// object has to be a tree
	if (!tree->tree) {
		return -ENOENT;
	}
	return git_tree_entrycount(tree->tree);
}

int gitfs_get_tree_entry(struct gitfs_object ** entry, struct gitfs_object * tree, int index)
{
	if (!tree->tree) {
		// not a tree
		return -ENOENT;
	}
	git_tree_entry * git_entry = (git_tree_entry *) git_tree_entry_byindex(tree->tree, index);
	if (!git_entry) {
		fprintf(stderr, "No entry in tree for index %d\n", index);
		return -ENOENT;
	}
	// got the entry
	entry[0] = calloc(1, sizeof(gitfs_object));
	entry[0]->tree_entry = git_entry;
	return 0;
}

char * gitfs_get_name(struct gitfs_object * object)
{
	char * name = object->name;
	if (name)
		// already had it, no need to get it again
		goto end;
	if (object->tree_entry) {
		name = strdup(git_tree_entry_name(object->tree_entry));
		goto end;
	}
end:
	object->name = name;
	return name;
}

void gitfs_shutdown()
{
	if (gitfs_info.revision)
		git_object_free(gitfs_info.revision);
	// going out, for the time being
	git_libgit2_shutdown();
}


