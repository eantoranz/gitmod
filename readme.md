# gitmod
a [FUSE](https://github.com/libfuse/libfuse)-based (at least for the time being) FS.

## What it does
**gitmod** can be used to virtually display the contents of
revisions/tags/branches on a read-only mount-point. No need
to have the files of the project on the FS. Just having
a bare repo (somewhere on the computer) should suffice to make the
files show up at a desired mount point.

# Features
At the moment it does the very basics:
- Lists files in the tree on the mount point
- Gets contents of files
- Permissions of executables are kept
- Tracks if the published *treeish* moves and updates accordingly

## Dependencies
- [libgit2](https://github.com/libgit2/libgit2)
- [FUSE](https://github.com/libfuse/libfuse)
- [glib](https://github.com/GNOME/glib)
- [CUnit](http://cunit.sourceforge.net/)

## How to compile
Run:

    make

That will generate a binary called `gitmod`

## Options:
It supports the standard options provided by [FUSE](https://github.com/libfuse/libfuse).
It also provides a few more options

- **--treeish**: use to specify which treeish to track (branch/tag/revision). Default: **HEAD**
- **--repo**: use to specify which repo to expose contents from. Default: **./**

Check more options and details with

    ./gitmod -h

## Examples

*Mount* the contents of a web project from branch **main** of a repo in **/home/user/project** on **apache** (*standard debian layout*).

    sudo ./gitmod --repo=/home/user/project --branch=main -o allow_other -o uid=$( id -u www-data ) /var/www/html
    
## Performance
The **--kim** (keep in memory). This option will force **gitmod** to keep objects that are
loaded from the git repo in memory. This option allows for a 10x throughput improvement in my computer.

## Debugging
You can run **make** like this to compile with debug output information

    DEBUG=1 make

This will make gitmod send information to stdout when running.
**Important**: might require running **gitmod** with FUSE option **-f** so that the process does not go into the background.

## building packages

### debian
There is a script called `scripts/build-debian` which can be used to build debian packages. It will use [Docker](https://hub.docker.com/_/debian)
to build the package. Call the script without parameters to see what parameters it expects when you call it.

## Copyright / Licensing
Copyright 2020-2024 Edmundo Carmona Antoranz. Released under the terms of [GPLv2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).

## IMPORTANT
This is very alpha software at the moment so:
- **DO NOT USE IT IN PRODUCTION**
- **USE AT YOUR OWN RISK**
