#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -ex

# Script that will take care building/creating the package
apt-get update
cat "$REQUIREMENTS_FILE" | xargs apt-get install -q -y build-essential devscripts debhelper git

mkdir /root/DEBBUILD

# let's create the package every single time so we make sure we are _not_ working on a preexisting (and potentially busted!!!) file
git config --global --add safe.directory /mnt/work # so that git can work without complaining
git archive --format=tar.gz --prefix=gitmod-"$VERSION"/ -o gitmod-$VERSION.tar.gz $COMMITTISH
cp gitmod-$VERSION.tar.gz /root/DEBBUILD/gitmod_$VERSION.orig.tar.gz

cd /root/DEBBUILD
tar zxvf gitmod_$VERSION.orig.tar.gz
cd gitmod-$VERSION
mv packages/debian debian
debuild -us -uc

TARGET_DIR=/mnt/work/packages/files/debian/$DEBIAN_TAG/$VERSION

if [ ! -d $TARGET_DIR ]; then
	mkdir -p $TARGET_DIR
fi

cp -v ../*.deb $TARGET_DIR

cd /mnt/work

echo Finished building packages for debian $DEBIAN_TAG for gitmod version $VERSION from committish $COMMITTISH
echo Packages coming out of the build are in packages/files/debian/$DEBIAN_TAG:

ls -l packages/files/debian/$DEBIAN_TAG/$VERSION

echo Thanks for using gitmod debian packager.
