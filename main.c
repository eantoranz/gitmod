/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#define FUSE_USE_VERSION 31

#include "include/gitfstrack.h"
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

static void *gitfs_fs_init(struct fuse_conn_info *conn,
                        struct fuse_config *cfg)
{
        (void) conn;
	printf("Running gitfs_init(...)\n");
	cfg->kernel_cache = 0; // TODO can use cache if working from a revision or tags (cause they are not _supposed_ to move, right?)
        gitfs_info.uid = cfg->set_uid;
	gitfs_info.gid = cfg->set_gid;
        return NULL;
}

static int gitfs_getattr(const char *path, struct stat *stbuf,
                         struct fuse_file_info *fi)
{
        (void) fi;
        int res = 0;


	printf("Running gitfs_getattr(\"%s\", ...)\n", path);

	struct gitfs_object * object = gitfs_get_object(path, 1);
	if (!object) {
		fprintf(stderr, "gitfs_getattr: Could not find an object for path %s\n", path);
		return -ENOENT;
	}

        memset(stbuf, 0, sizeof(struct stat));
	stbuf->st_atime = gitfs_info.time;
	stbuf->st_ctime = gitfs_info.time;
	stbuf->st_mtime = gitfs_info.time;
	stbuf->st_uid = gitfs_info.uid;
	stbuf->st_gid = gitfs_info.gid;
	stbuf->st_nlink = gitfs_get_num_entries(object);
	enum gitfs_object_type object_type = gitfs_get_type(object);
        if (object_type == GITFS_TREE) { // this will depend on the type of object
                stbuf->st_mode = S_IFDIR | 0555; // mode is always 0 for trees
		stbuf->st_nlink+=2;
	} else if (object_type == GITFS_BLOB){
		stbuf->st_mode = S_IFREG | gitfs_get_mode(object);
		stbuf->st_size = gitfs_get_size(object);
	} else {
		res = -ENOENT;
	}
	gitfs_dispose(object);

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

	struct gitfs_object * dir_node = gitfs_get_object(path, 0);
        if (!dir_node || gitfs_get_type(dir_node) != GITFS_TREE) {
		fprintf(stderr, "gitfs_readdir: Could not find an object for path %s (or it's not a tree)\n", path);
		if (dir_node)
			gitfs_dispose(dir_node);
                return -ENOENT;
	}

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	int num_items = gitfs_get_num_entries(dir_node);
	struct gitfs_object * entry;
	for (int i=0; i < num_items; i++) {
		entry = gitfs_get_tree_entry(dir_node, i, 0);
		if (entry) {
			char * name = gitfs_get_name(entry);
			if (name)
				filler(buf, name, NULL, 0, 0);
			gitfs_dispose(entry);
		}
	}
	gitfs_dispose(dir_node);

        return 0;
}

static int gitfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
        size_t len;
        (void) fi;
	struct gitfs_object * object = gitfs_get_object(path, 0);
	if (!object || gitfs_get_type(object) != GITFS_BLOB) {
		fprintf(stderr, "gitfs_read: Could not find an object for path %s (or it's not a blob)\n", path);
		if (object)
			gitfs_dispose(object);
		return -ENOENT;
	}
	
	len = gitfs_get_size(object);
	const char * contents = gitfs_get_content(object);
        if (offset < len) {
                if (offset + size > len)
                        size = len - offset;
                memcpy(buf, contents + offset, size);
        } else
                size = 0;
	
	gitfs_dispose(object);
        return size;
}

static void gitfs_destroy()
{
	printf("Running gitfs_destroy()\n");
	gitfs_shutdown();

}

static const struct fuse_operations gitfs_oper = {
        .init           = gitfs_fs_init,
	.getattr        = gitfs_getattr,
	.readdir        = gitfs_readdir,
	.read           = gitfs_read,
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

	ret = gitfs_init(options.repo_path, options.treeish);

	if (ret)
		gitfs_shutdown();
	else
		fuse_main(args.argc, args.argv, &gitfs_oper, NULL);

	return ret;
}

