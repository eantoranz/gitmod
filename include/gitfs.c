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
	git_blob * blob;
	char * name; // local name, _not_ fullpath
	char * path; // full path
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
	gitfs_info.time = git_commit_time((git_commit *) gitfs_info.revision);

	printf("Using tree %s as the root of the mount point\n", git_oid_tostr_s(git_tree_id(gitfs_info.root_tree)));
end:
	return ret;
}

void gitfs_dispose(struct gitfs_object * object)
{
	if (object->tree && object->tree != gitfs_info.root_tree)
		git_tree_free(object->tree);
	if (object->blob)
		git_blob_free(object->blob);
	if (object->name)
		free(object->name);
	if (object->path)
		free(object->path);

	// finally
	free(object);
}

static struct gitfs_object * gitfs_get_object_from_git_tree_entry(git_tree_entry * gitfs_entry)
{
	struct gitfs_object * object = calloc(1, sizeof(gitfs_object));
	if (!object) {
		return NULL;
	}
	object->name = strdup(git_tree_entry_name(gitfs_entry));
	git_otype otype = git_tree_entry_type(gitfs_entry);
	int ret;
	switch (otype) {
	case GIT_OBJ_BLOB:
		ret = git_tree_entry_to_object((git_object **) &object->blob, gitfs_info.repo, gitfs_entry);
		break;
	case GIT_OBJ_TREE:
		ret = git_tree_entry_to_object((git_object **) &object->tree, gitfs_info.repo, gitfs_entry);
		break;
	default:
		ret = -ENOENT;
	}
	if (ret && object) {
		gitfs_dispose(object);
		object = NULL;
	}
	return object;
}

struct gitfs_object * gitfs_get_object(const char *path)
{
	int ret = 0;
	struct gitfs_object * object;
	if (!(strlen(path) && strcmp(path, "/"))) {
		// root tree
		object = calloc(1, sizeof(gitfs_object));
		object->tree = gitfs_info.root_tree;
		object->path = strdup("/");
		object->name = strdup("/");
		return object;
	}

	struct git_tree_entry * tree_entry;
	ret = git_tree_entry_bypath(&tree_entry, gitfs_info.root_tree, path + (path[0] == '/' ? 1 : 0));
	if (ret) {
		fprintf(stderr, "Could not find the object for the path %s\n", path);
		return NULL;
	}
	
	object = gitfs_get_object_from_git_tree_entry(tree_entry);
	if (object)
		object->path = strdup(path);
	if (tree_entry)
		git_tree_entry_free(tree_entry);
	return object;
}

enum gitfs_object_type gitfs_get_type(struct gitfs_object * object) {
	if (object->tree) {
		return GITFS_TREE;
	}
	if (object->blob) {
		return GITFS_BLOB;
	}
	return GITFS_UNKNOWN;
}

int gitfs_get_num_entries(struct gitfs_object * object)
{
	enum gitfs_object_type type = gitfs_get_type(object);
	int res;
	switch (type) {
	case GITFS_BLOB:
		res = 1;
		break;
	case GITFS_TREE:
		res = git_tree_entrycount(object->tree);
		break;
	default:
		res = -ENOENT;
	}
	return res;
}

int gitfs_get_size(struct gitfs_object * object)
{
	int res;
	
	if (object->blob)
		res = git_blob_rawsize(object->blob);
	else if (object->tree)
		res = gitfs_get_num_entries(object);
	else
		res = -ENOENT;
	
	return res;
}

struct gitfs_object * gitfs_get_tree_entry(struct gitfs_object * tree, int index)
{
	if (!tree->tree)
		// not a tree
		return NULL;
	git_tree_entry * git_entry = (git_tree_entry *) git_tree_entry_byindex(tree->tree, index); // no need to dispose of manually
	if (!git_entry) {
		fprintf(stderr, "No entry in tree for index %d\n", index);
		return NULL;
	}
	// got the entry
	struct gitfs_object * entry = gitfs_get_object_from_git_tree_entry(git_entry);
	if (!entry)
		// could not create the object
		return NULL;
	// TODO get full path
	
	return entry;
}

char * gitfs_get_name(struct gitfs_object * object)
{
	return object->name;
}

const char * gitfs_get_content(struct gitfs_object * object)
{
	if (!object->blob)
		return NULL;
	return git_blob_rawcontent(object->blob);
}

void gitfs_shutdown()
{
	if (gitfs_info.revision)
		git_object_free(gitfs_info.revision);
	// going out, for the time being
	git_libgit2_shutdown();
}
