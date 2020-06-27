Copyright 2020 Edmundo Carmona Antoranz
Released under the terms of GPLv2

Hi!

Thanks for taking the time to read this document.

What I want to do is create a fuse-based (at least for the time being)
FS so that content from branches, tags or revisions can be shown.

At the moment it does the very basics:
- Lists files in the tree on the mount point
- Get content of files

Dependencies
- libgit2
- cunit1
- fuse3

How to compile it:
./build.sh

That will generate a binary called gitfstrack

Usage:
- It supports the standard options provided by fuse. It also has 2 more options:

--treeish: use to specify which treeish to track (branch/tag/revision). Default is HEAD
--repo: use to specify which repo to expose contents from. Default: ./


This is very alpha software at the moment so:
*** DO NOT USE IT IN PRODUCTION ***
*** USE AT YOUR OWN RISK ***

