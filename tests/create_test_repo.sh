#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -e

ROOT_DIR=$( git rev-parse --show-toplevel )
if [ "$PWD" != "$ROOT_DIR" ]; then
  cd "$ROOT_DIR"
fi

TEST_REPO_DIR=tests/test_repo

if [ -d $TEST_REPO_DIR ]; then
  echo Removing preexiting test_repo
  rm -fR $TEST_REPO_DIR
fi
echo Creating test repo
git init --quiet -b test-main $TEST_REPO_DIR
cd $TEST_REPO_DIR

# first commit
export GIT_AUTHOR_NAME="Fulanito D'Tal"
export GIT_AUTHOR_EMAIL=fulanito@dt.al
export GIT_AUTHOR_DATE="1500000000 -0100"
export GIT_COMMITTER_NAME="Menganito D'Cual"
export GIT_COMMITTER_EMAIL=menganito@dcu.al
export GIT_COMMITTER_DATE="1600000000 -0200"

cat > readme.txt <<EOF
This repo will be used to perform both unit tests and
integration(?) tests for gitmod.

There will be a few commits so that we can move a reference around
and see that gitmod behaves correctly displaying the content we
expect on the mount points.
EOF
git add readme.txt

cat > hello-world.sh <<EOF
#!/bin/bash

echo Hello world
EOF
chmod +x hello-world.sh
git add hello-world.sh

mkdir some-dir
cat > some-dir/sample-file.txt <<EOF
Here is a sample text file
Nothing important. Need to make sure we can access a directory
EOF
git add some-dir/sample-file.txt

git commit -q -m "First commit: Adding readme.txt hello-world.sh and a directory with a file"

# second commit
export GIT_AUTHOR_NAME="Don Quixote"
export GIT_AUTHOR_EMAIL=dlamancha@fake-books.com
export GIT_AUTHOR_DATE="1700000000 -0300"
export GIT_COMMITTER_NAME=Rapunzel
export GIT_COMMITTER_EMAIL=wigjunkie@oldstorytales.com
export GIT_COMMITTER_DATE="1800000000 -0400"

cat > cowsay.txt <<EOF
 ___________
< git rules >
 -----------
        \   ^__^
         \  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||

Generated with cowsay
EOF
git add cowsay.txt
git commit -q -m "Second commit: added a simple cowsay file"
git tag intermediate

# third commit
export GIT_AUTHOR_NAME=tux
export GIT_AUTHOR_EMAIL=tux@penguins.com
export GIT_AUTHOR_DATE="1900000000 -0500"
export GIT_COMMITTER_NAME=Linus
export GIT_COMMITTER_EMAIL=nottorvalds@localhost
export GIT_COMMITTER_DATE="2000000000 -0600"

cat > tux.txt <<EOF
 _____________
< Linux rules >
 -------------
   \
    \
        .--.
       |o_o |
       |:_/ |
      //   \ \
     (|     | )
    / \_   _/ \
    \___)=(___/

Also cowsay
EOF

git add tux.txt
git commit -q -m "Third commit: tux saying that linux rules (What a shock!!!)"
