/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#define FUSE_USE_VERSION 31

#include "include/gitfs.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fuse.h>

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

static void *gitfs_init(struct fuse_conn_info *conn,
                        struct fuse_config *cfg)
{
        (void) conn;
	printf("Running gitfs_init(...)\n");
        cfg->kernel_cache = 1; // TODO consider what will need to be done when we track a moving branch
        return NULL;
}

static int gitfs_getattr(const char *path, struct stat *stbuf,
                         struct fuse_file_info *fi)
{
        (void) fi;
        int res = 0;

	printf("Running gitfs_getattr(\"%s\", ...)\n", path);

        memset(stbuf, 0, sizeof(struct stat));
        if (!strcmp(path, "/")) { // root node is a directory
		int items;
		printf("It's the root node\n");
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
                stbuf->st_mode = S_IFDIR | 0755;
                items = gitfs_get_num_items(path);
		if (items < 0) {
			stbuf->st_nlink = 2;
			return -ENOENT;
		}
		stbuf->st_nlink = items + 2;
	} else
                res = -ENOENT;

        return res;
}

static int gitfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags)
{
        (void) offset;
        (void) fi;
        (void) flags;

	printf("Running gitfs_readdir(\"%s\", ...)\n", path);

        if (strcmp(path, "/") != 0) // TODO only working on root node for the time being
                return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	int num_items = gitfs_get_num_items(path);
	for (int i=0; i < num_items; i++) {
		char * item_path = gitfs_get_entry_name(path, i);
		if (item_path) {
			filler(buf, item_path, NULL, 0, 0);
			free(item_path);
		}
	}

        return 0;
}

static void gitfs_destroy()
{
	printf("Running gitfs_destroy()\n");
	gitfs_git_shutdown();

}

static const struct fuse_operations gitfs_oper = {
        .init           = gitfs_init,
	.getattr        = gitfs_getattr,
	.readdir        = gitfs_readdir,
	.destroy        = gitfs_destroy,
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

	ret = gitfs_git_init(options.repo_path, options.treeish);

	if (ret)
		gitfs_git_shutdown();
	else
		fuse_main(args.argc, args.argv, &gitfs_oper, NULL);

	return ret;
}

