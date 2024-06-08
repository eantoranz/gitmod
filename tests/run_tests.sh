#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -e

ROOT_DIR=$( git rev-parse --show-toplevel )
if [ "$PWD" != "$ROOT_DIR" ]; then
  cd "$ROOT_DIR"
fi

echo IMPORTANT: Please, make sure that you have clean/built the project before running the tests.

./tests/create_test_repo.sh

MD5_3RD_COMMIT=15e95b9920370979d78e691435d37233
MD5_2ND_COMMIT=808035e0f0b97e541daeae94e7f6f90a
TREE_2ND_COMMIT=65517b96a487fbf59775aaefae3f8faff634ae79
MD5_1ST_COMMIT=89dc4613ac19ee2dfb3c15529b25ab1d


./tests/unit_tests || ( echo Unit tests failed; exit 1 )

echo Unit tests were successful. Going for the real-life tests, hold on tight.

echo

if [ ! -d tests/mount-point ]; then
  echo Creating test mount point
  mkdir tests/mount-point
fi

cd tests/test_repo
# the only branch present is test-main, working tree is at the tip
git branch moving
cd - > /dev/null

# First test
echo First test
./bin/gitmod --treeish=moving --repo=tests/test_repo tests/mount-point
echo Waiting for gitmod to get setup
sleep 2 # give it some time to get itself setup
MD5=$( tar c tests/mount-point | md5sum - | awk '{print $1;}' )
if [ "$MD5" != "$MD5_3RD_COMMIT" ]; then
  echo Was expecting a different MD5 value. Got $MD5
  echo Unexpected content checking the tip of branch \"moving\" in mount point
  ls -l tests/mount-point
  echo Unmounting test mount point
  umount tests/mount-point
  exit 1
fi
umount tests/mount-point
echo Success
echo

# Second test
echo Second test
cd tests/test_repo
git branch -f moving test-main~2 # taking it back to the first commit
cd - > /dev/null

./bin/gitmod --treeish=moving --repo=tests/test_repo tests/mount-point
echo Waiting for gitmod to get setup
sleep 2 # give it some time to get itself setup
MD5=$( tar c tests/mount-point | md5sum - | awk '{print $1;}' )
if [ "$MD5" != "$MD5_1ST_COMMIT" ]; then
  echo Was expecting a different MD5 value. Got $MD5
  echo Unexpected content checking the tip of branch \"moving\" in mount point
  ls -l tests/mount-point
  echo Unmounting test mount point
  umount tests/mount-point
  exit 1
fi
# do not unmount, third test will check that the mount point is refreshed live
echo Success
echo

# Third test
echo Third test
cd tests/test_repo
BEFORE=$( git rev-parse moving )
echo Moving pointer, content must be updated
git branch -f moving test-main # taking it back to the tip of test-main
AFTER=$( git rev-parse moving )
cd - > /dev/null
if [ "$BEFORE" == "$AFTER" ]; then
  echo TEST BUG: pointer for branch \"moving\" did not move. Check the test.
  uhmount test/mount-point
  exit 1
fi
# we wait a little bit to see that content has been refreshed
sleep 1

MD5=$( tar c tests/mount-point | md5sum - | awk '{print $1;}' )
if [ "$MD5" != "$MD5_3RD_COMMIT" ]; then
  echo Was expecting a different MD5 value. Got $MD5
  echo Unexpected content checking the tip of branch \"moving\" in mount point
  ls -l tests/mount-point
  echo Unmounting test mount point
  umount tests/mount-point
  exit 1
fi
umount tests/mount-point
echo Success
echo

# fourth test, can use a fixed commit id as treeish
echo Fourth test
./bin/gitmod --treeish=test-main~ --repo=tests/test_repo tests/mount-point
echo Waiting for gitmod to get setup
sleep 2 # give it some time to get itself setup
MD5=$( tar c tests/mount-point | md5sum - | awk '{print $1;}' )
if [ "$MD5" != "$MD5_2ND_COMMIT" ]; then
  echo Was expecting a different MD5 value. Got $MD5
  echo Unexpected content checking the tip of branch \"moving\" in mount point
  ls -l tests/mount-point
  echo Unmounting test mount point
  umount tests/mount-point
  exit 1
fi
umount tests/mount-point
echo Success
echo

# fifth test, if fixed, the treeish can move and the result won't change
echo Fifth test
./bin/gitmod --treeish=moving --repo=tests/test_repo --fix tests/mount-point
echo Waiting for gitmod to get setup
sleep 2 # give it some time to get itself setup
MD5=$( tar c tests/mount-point | md5sum - | awk '{print $1;}' )
if [ "$MD5" != "$MD5_3RD_COMMIT" ]; then
  echo Was expecting a different MD5 value. Got $MD5
  echo Unexpected content checking the original content using \"moving\" as a fixed treeish
  ls -l tests/mount-point
  echo Unmounting test mount point
  umount tests/mount-point
  exit 1
fi
# moving pointer
echo Moving pointer, content must remain the same
cd tests/test_repo
BEFORE=$( git rev-parse moving )
git branch -f moving test-main~2 # taking it back to the first commit
AFTER=$( git rev-parse moving )
cd - > /dev/null
if [ "$BEFORE" == "$AFTER" ]; then
  echo TEST BUG: pointer for branch \"moving\" did not move. Check the test.
  uhmount test/mount-point
  exit 1
fi
sleep 1
# checking result, it must be the same as before
MD5=$( tar c tests/mount-point | md5sum - | awk '{print $1;}' )
if [ "$MD5" != "$MD5_3RD_COMMIT" ]; then
  echo Was expecting a different MD5 value. Got $MD5
  echo Unexpected content checking fixed content after treeish
  ls -l tests/mount-point
  echo Unmounting test mount point
  umount tests/mount-point
  exit 1
fi
umount tests/mount-point
echo Success
echo


echo Tests were successful