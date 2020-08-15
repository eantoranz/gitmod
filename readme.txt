Copyright 2020 Edmundo Carmona Antoranz
Released under the terms of GPLv2

gitmod: a FUSE-based (at least for the time being) FS.

gitmod can be used to virtually display the contents of
revisions/tags/branches on a read-only mount-point.

At the moment it does the very basics:
- Lists files in the tree on the mount point
- Get content of files
- Permissions of executables are kept

Dependencies
- libgit2
- cunit1
- fuse3

How to compile it:
Run: make

That will generate a binary called gitmod

USAGE:
It supports the standard options provided by fuse.
It also provides a few more options
--treeish: use to specify which treeish to track (branch/tag/revision). Default is HEAD
--repo: use to specify which repo to expose contents from. Default: ./

Check more options and details with ./gitmod -h

PERFORMANCE:
The --kim option allows a 10x throughput improvement in my computer.

DEBUGGING:
You can run make like this to compile with debug information
DEBUG=1 make

This will make gitmod send information to stdout when running.
Important: might require run gitmod with FUSE option -f


IMPORTANT:
This is very alpha software at the moment so:
*** DO NOT USE IT IN PRODUCTION ***
*** USE AT YOUR OWN RISK ***

