/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#define FUSE_USE_VERSION 31

#include <assert.h>
#include <git2.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <fuse.h>

git_repository *repo;
// tree of the root of the treeish that wants to be used as the root for the mount point
git_tree *root_tree; // TODO if following a branch, this value could change... will be fixed for the time being

static struct options {
	const char *repo_path; // path to git repo,
	const char *treeish; // treeish to use
	int show_help;
} options;

#define OPTION(t, p) \
	{ t, offsetof(struct options, p), 1 }

static const struct fuse_opt option_spec[] = {
	OPTION("--repo=%s", repo_path),
	OPTION("--treeish=%s", treeish),
	OPTION("--help", show_help),
	OPTION("-h", show_help),
	FUSE_OPT_END
};

static void show_help(const char *progname)
{
        printf("usage: %s [options] <mountpoint>\n\n", progname);
        printf("File-system specific options:\n"
               "    --repo=<s>          Path to the git repo\n"
               "                        (default: \".\", current directory)\n"
	       "    --treeish=<s>       Treeish to use on the root of the mount point\n"
	       "                        It can be a branch, a revision or a tag.\n"
	       "                        (default: HEAD)\n"
               "\n");
}

int main(int argc, char *argv[])
{
        int ret;
        struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	git_object * treeish = NULL;

        /* Set defaults -- we have to use strdup so that
           fuse_opt_parse can free the defaults if other
           values are specified */
        options.repo_path = strdup(".");
	options.treeish = strdup("HEAD");

        /* Parse options */
        if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
                return 1;

        /* When --help is specified, first print our own file-system
           specific help text, then signal fuse_main to show
           additional help (by adding `--help` to the options again)
           without usage: line (by setting argv[0] to the empty
           string) */
        if (options.show_help) {
                show_help(argv[0]);
                assert(fuse_opt_add_arg(&args, "--help") == 0);
                args.argv[0][0] = '\0';
        }

	// check that we can open the git repository
	git_libgit2_init();
	ret = git_repository_open(&repo, options.repo_path);
	if (ret) {
		// there was an error opening the repository
		fprintf(stderr, "There was an error opening the git repo at %s\n", options.repo_path);
		goto end;
	}

	printf("Successfully opened repo at %s\n", git_repository_commondir(repo));
	ret = git_revparse_single(&treeish, repo, options.treeish);
	if (ret) {
		fprintf(stderr, "There was error parsing the threeish %s on the repo\n", options.treeish);
		goto end;
	}

	printf("Successfully parsed treeish %s\n", options.treeish);
	ret =  git_commit_tree(&root_tree, (git_commit *) treeish);
	if (ret) {
		fprintf(stderr, "Could not find tree object for the revision\n");
		goto end;
	}

	printf("Using tree %s as the root of the mount point\n", git_oid_tostr_s(git_tree_id(root_tree)));
end:
	if (treeish)
		git_object_free(treeish);
	// going out, for the time being
	git_libgit2_shutdown();
	return ret;
}


