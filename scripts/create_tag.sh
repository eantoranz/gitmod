#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2


set -e

if [ $# -lt 1 ]; then
	echo Did not specify a tag to use
	exit 1
fi

TAG=${1#v}

# make sure worktree is clean
if [ $( git status --short | wc -l ) -ne 0 ]; then
	echo Working tree is not clean. Can\'t create tag
	exit 1
fi

echo Adding tag to debian changelog
export DEBFULLNAME="$( git config user.name )"
export DEBEMAIL="$( git config user.email )"
export VISUAL=/bin/true EDITOR=/bin/true
pushd packages
	# debian adjustments
	dch -v $TAG-1 Released gitmod $TAG
	dch -r $TAG-1
	# rpm adjustment
	# is there an automated way to do this?
	sed -i "s/^Version:.*/Version:        $TAG/" rpm/gitmod.spec
	sed -i "s/^Release:.*/Release:        1%{?dist}/" rpm/gitmod.spec
	sed -i "s/%changelog.*/%changelog\n* $( LANG=en date +"%a %b %d %Y" ) $( git config user.name )\n- Released gitmod $TAG/" rpm/gitmod.spec
popd

git add packages/debian/changelog
git add packages/rpm/gitmod.spec

echo $TAG > .gitmod_VERSION.txt
git add .gitmod_VERSION.txt

echo Ready to create the tag v$TAG.
echo This is your last chance to modify the information for creating the deb and rpm files.
echo
while true; do
	echo When you are ready, make sure that all changes have been added to index. If that is not the case,
	echo you will be given a new opportunity to do it.
	echo
	echo run \"exit 0\" to proceed and create the tag or \"exit 1\" to abort tag creation.

	/bin/bash || ( echo Aborting tag creation; git checkout HEAD -- .; exit 1 )

	# there should be no change between index and working tree
	if [ $( git diff --name-only | wc -l ) -eq 0 ]; then
		break
	fi
	echo There are changes that are not in index.
	git status
	echo Try again.
	echo
done

git commit -m "Creating gitmod tag v$TAG"
git tag -f v$TAG

echo $TAG.dirty > .gitmod_VERSION.txt
git add .gitmod_VERSION.txt

git commit -m "Setting up dirty version of $TAG"
