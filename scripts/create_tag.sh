#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2


set -x

if [ $# -lt 1 ]; then
	echo Did not specify a tag to use
	exit 1
fi

TAG=$1

echo Adding tag to debian changelog
export DEBFULLNAME="$( git config user.name )"
export DEBEMAIL="$( git config user.email )"
export VISUAL=/bin/true EDITOR=/bin/true
pushd packages
	dch -v $TAG-1 Released gitmod $TAG
	dch -r $TAG-1
popd

git add packages/debian/changelog

echo $TAG > .gitmod_VERSION.txt
git add .gitmod_VERSION.txt

git commit -m "Creating gitmod tag v$TAG"
git tag -f v$TAG

echo $TAG.dirty > .gitmod_VERSION.txt
git add .gitmod_VERSION.txt

git commit -m "Setting up dirty version of $TAG"
