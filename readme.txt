Copyright 2020 Edmundo Carmona Antoranz
Released under the terms of GPLv2

Hi!

Thanks for taking the time to read this document.

What I want to do is create a fuse-based (at least for the time being)
FS so that content from branches, tags or revisions can be shown.

At the moment it does the very basics:
- Lists files in the tree on the mount point
- Get content of files

Ideally, it should be able to _track_ branches but will
have to work on that cause it's not working at the moment.

Dependencies
- libgit2
- cunit1
- fuse3

This is very alpha software at the moment so:
***DO NOT USE IT IN PRODUCTION***


