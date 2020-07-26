/*
 * Copyright 2020 Edmundo Carmona Antoranz
 * Released under the terms of GPLv2
 */

#define FUSE_USE_VERSION 35

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fuse.h>
#include "object.h"
#include "gitmod.h"

static struct options {
	const char *repo_path; // path to git repo,
	const char *treeish; // treeish to use
	const int allow_exec; // allow exec bit for files (default: 0)
	const int fix; // do not track changes of references
	int root_tree_delay; // in milliseconds (default: 100)
	int show_help;
	int debug;
} options;

#define OPTION(t, p) \
	{ t, offsetof(struct options, p), 1 }

static const struct fuse_opt option_spec[] = {
	OPTION("--repo=%s", repo_path),
	OPTION("--treeish=%s", treeish),
	OPTION("--allow-exec", allow_exec),
	OPTION("-x", allow_exec),
	OPTION("--refresh-delay=%d", root_tree_delay),
	OPTION("--fix", fix),
	OPTION("--debug", debug),
	OPTION("--help", show_help),
	OPTION("-h", show_help),
	FUSE_OPT_END
};

static void *gitmod_fs_init(struct fuse_conn_info *conn,
                        struct fuse_config *cfg)
{
        (void) conn;
	if (options.debug)
		printf("Running gitmod_init(...)\n");
	cfg->kernel_cache = options.fix;
        gitmod_info.uid = cfg->set_uid;
	gitmod_info.gid = cfg->set_gid;
        return NULL;
}

static int gitmod_fs_getattr(const char *path, struct stat *stbuf,
                         struct fuse_file_info *fi)
{
        (void) fi;
        int res = 0;


	if (options.debug)
		printf("Running gitmod_getattr(\"%s\", ...)\n", path);

	gitmod_object * object = gitmod_get_object(path, 1);
	if (!object) {
		fprintf(stderr, "gitmod_getattr: Could not find an object for path %s\n", path);
		return -ENOENT;
	}

        memset(stbuf, 0, sizeof(struct stat));
	stbuf->st_atime = gitmod_info.root_tree->time;
	stbuf->st_ctime = gitmod_info.root_tree->time;
	stbuf->st_mtime = gitmod_info.root_tree->time;
	stbuf->st_uid = gitmod_info.uid;
	stbuf->st_gid = gitmod_info.gid;
	stbuf->st_nlink = gitmod_object_get_num_entries(object);
	enum gitmod_object_type object_type = gitmod_object_get_type(object);
        if (object_type == GITMOD_OBJECT_TREE) { // this will depend on the type of object
                stbuf->st_mode = S_IFDIR | 0555; // mode is always 0 for trees
		stbuf->st_nlink+=2;
	} else if (object_type == GITMOD_OBJECT_BLOB){
		stbuf->st_mode = S_IFREG |
				 (gitmod_object_get_mode(object) & (options.allow_exec ? 0777 : 0666));
		stbuf->st_size = gitmod_object_get_size(object);
	} else
		res = -ENOENT;
	gitmod_dispose_object(&object);

        return res;
}

static int gitmod_fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags)
{
        (void) offset;
        (void) fi;
        (void) flags;

	if (options.debug)
		printf("Running gitmod_readdir(\"%s\", ...)\n", path);

	gitmod_object * dir_node = gitmod_get_object(path, 0);
        if (!dir_node || gitmod_object_get_type(dir_node) != GITMOD_OBJECT_TREE) {
		fprintf(stderr, "gitmod_readdir: Could not find an object for path %s (or it's not a tree)\n", path);
		if (dir_node)
			gitmod_dispose_object(&dir_node);
                return -ENOENT;
	}

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	int num_items = gitmod_object_get_num_entries(dir_node);
	gitmod_object * entry;
	for (int i=0; i < num_items; i++) {
		entry = gitmod_get_tree_entry(dir_node, i, 0);
		if (entry) {
			char * name = gitmod_object_get_name(entry);
			if (name)
				filler(buf, name, NULL, 0, 0);
			gitmod_dispose_object(&entry);
		}
	}
	gitmod_dispose_object(&dir_node);

        return 0;
}

static int gitmod_fs_open(const char * path, struct fuse_file_info *fi)
{
	int ret = 0;
	gitmod_object * object = gitmod_get_object(path, 0);
	if (!object || gitmod_object_get_type(object) != GITMOD_OBJECT_BLOB) {
		fprintf(stderr, "gitmod_fs_open: Could not find an object for path %s (or it's not a blob)\n", path);
		if (object)
			gitmod_dispose_object(&object);
		ret = -ENOENT;
	} else
		fi->fh = (uint64_t) object;
	return ret;
}

static int gitmod_fs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
        size_t len;
        (void) fi;
	gitmod_object * object = (gitmod_object *) fi->fh;
	
	len = gitmod_object_get_size(object);
	const char * contents = gitmod_object_get_content(object);
        if (offset < len) {
                if (offset + size > len)
                        size = len - offset;
                memcpy(buf, contents + offset, size);
        } else
                size = 0;
	
        return size;
}

static int gitmod_fs_release(const char * path, struct fuse_file_info *fi)
{
	gitmod_object * object = (gitmod_object *) fi->fh; 
	gitmod_dispose_object(&object);
	return 0;
}

static void gitmod_fs_destroy()
{
	if (options.debug)
		printf("Running gitmod_destroy()\n");
	gitmod_shutdown();

}

static const struct fuse_operations gitmod_oper = {
        .init           = gitmod_fs_init,
	.getattr        = gitmod_fs_getattr,
	.readdir        = gitmod_fs_readdir,
	.open           = gitmod_fs_open,
	.read           = gitmod_fs_read,
	.release        = gitmod_fs_release,
	.destroy        = gitmod_fs_destroy,
};


static void show_help(const char *progname)
{
        printf("usage: %s [options] <mountpoint>\n\n", progname);
        printf("File-system specific options:\n"
               "    --repo=<s>             Path to the git repo\n"
               "                           (default: \".\", current directory)\n"
	       "    --treeish=<s>          Treeish to use on the root of the mount point\n"
	       "                           It can be a branch, a revision or a tag.\n"
	       "                           (default: HEAD)\n"
               "    -x   --allow-exec      Allow execution flag on files\n"
               "                           (default: no)\n"
	       "    --fix                  Do not track changes in references.\n"
	       "                           Useful if using a tag\n"
	       "    --refresh-delay=<d>    Milliseconds between checks for movement of reference\n"
	       "                           (default: 100 milliseconds. 0 means it's a tight loop)\n"
               "    --debug                Show some debugging messages\n"
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
	options.root_tree_delay = ROOT_TREEE_MONITOR_DEFAULT_DELAY;

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
        } else {
		gitmod_info.fix = options.fix;
		gitmod_info.root_tree_delay = options.root_tree_delay;
		ret = gitmod_init(options.repo_path, options.treeish);

		if (ret)
			gitmod_shutdown();
	}
	
	if (!ret)
		fuse_main(args.argc, args.argv, &gitmod_oper, NULL);

	return ret;
}

