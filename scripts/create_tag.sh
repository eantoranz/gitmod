#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2


set -x

if [ $# -lt 1 ]; then
	echo Did not specify a tag to use
	exit 1
fi

TAG=${1#v}

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

git commit -m "Creating gitmod tag v$TAG"
git tag -f v$TAG

echo $TAG.dirty > .gitmod_VERSION.txt
git add .gitmod_VERSION.txt

git commit -m "Setting up dirty version of $TAG"
