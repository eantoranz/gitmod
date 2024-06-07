#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -ex

# Script that will take care building/creating the package
mkdir /root/DEBBUILD

# let's create the package every single time so we make sure we are _not_ working on a preexisting (and potentially busted!!!) file
git config --global --add safe.directory /mnt/work # so that git can work without complaining
git archive --format=tar.gz --prefix=gitmod-"$VERSION"/ -o gitmod-$VERSION.tar.gz $COMMITTISH -- Makefile src readme\*
cp gitmod-$VERSION.tar.gz /root/DEBBUILD/gitmod_$VERSION.orig.tar.gz

cd /root/DEBBUILD
tar zxvf gitmod_$VERSION.orig.tar.gz
cd gitmod-$VERSION
cp -R /mnt/work/packages/debian debian

# replace values in control.in file
LIBGIT2=$( apt-cache search -q libgit2-1 | awk '{print $1}' )
if [ "$LIBGIT2" == "" ]; then
	echo Could not find out what package to use as a dependency for libgit2
	exit 1
fi

LIBGLIB2_0=$( apt-cache search -q libglib2.0-0 | awk '{print $1}' )
if [ "$LIBGLIB2_0" == "" ]; then
	echo Could not find out what package to use as a dependency for libglib2.0-0
	exit 1
fi

echo libgit2 package: $LIBGIT2
echo libglib2.0 package: $LIBGLIB2_0

sed "s/%LIBGIT2%/$LIBGIT2/; s/%LIBGLIB2.0%/$LIBGLIB2_0/" debian/control.in > debian/control

debuild -us -uc

# making sure that the package can be installed
apt install -y ../*.deb || ( echo Failed installation test; exit 1 )
echo Package can be installed without issues

TARGET_DIR=packages/files/gitmod-$VERSION/$DISTRO-$DOCKER_TAG

if [ ! -d /mnt/work/$TARGET_DIR ]; then
	mkdir -p /mnt/work/$TARGET_DIR
fi

cp -v ../*.deb /mnt/work/$TARGET_DIR

cd /mnt/work

echo Finished building packages for $DISTRO $DOCKER_TAG for gitmod version $VERSION from committish $COMMITTISH
echo Packages coming out of the build are in $TARGET_DIR:

ls -l $TARGET_DIR

echo Thanks for using gitmod deb packager.
