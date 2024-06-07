#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -e

# Script that will take care building/creating the package

WORK_DIR="$PWD"

function get_distro_version_suffix {
	if [ -f /etc/lsb-release ]; then
		. /etc/lsb-release
		echo $DISTRIB_RELEASE
	elif [ -f /etc/debian_version ]; then
		cat /etc/debian_version | head -n 1 | sed 's/\./u/' # take the first line only, just in case
	else
		echo Could not find distro version suffix "(no lsb-release or debian_version file in /etc)" >&2
	fi
}

TARGET_DIR=packages/files/gitmod-$VERSION

if [ ! -d $TARGET_DIR ]; then
	mkdir -p $TARGET_DIR
fi

mkdir /root/DEBBUILD

# let's create the package every single time so we make sure we are _not_ working on a preexisting (and potentially busted!!!) file
git config --global --add safe.directory "$WORK_DIR" # so that git can work without complaining
git archive --format=tar.gz --prefix=gitmod-"$VERSION"/ -o $TARGET_DIR/gitmod-$VERSION.tar.gz $COMMITTISH -- Makefile src readme\*
cp $TARGET_DIR/gitmod-$VERSION.tar.gz /root/DEBBUILD/gitmod_$VERSION.orig.tar.gz

cd /root/DEBBUILD
tar zxvf gitmod_$VERSION.orig.tar.gz
cd gitmod-$VERSION
cp -R "$WORK_DIR"/packages/debian debian

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

# adjust version in changelog
# find out the short name of the distro to use in the version
DISTRO_VERSION_SUFFIX=$( get_distro_version_suffix )
if [ "$DISTRO_VERSION_SUFFIX" == "" ]; then
	# error message has been provided already in the function, no need to elaborate
	exit 1
fi

DISTRO_PACKAGE_VERSION_SUFFIX=${DISTRO:0:3}${DISTRO_VERSION_SUFFIX}
echo Package distro version suffix: $DISTRO_PACKAGE_VERSION_SUFFIX

sed -i "s/gitmod (\(.*\)) \(.*\)/gitmod (\1+$DISTRO_PACKAGE_VERSION_SUFFIX) \2/" debian/changelog

debuild -us -uc

# making sure that the package can be installed
apt install -y ../*.deb || ( echo Failed installation test; exit 1 )
echo Package can be installed without issues

cp -v ../*.deb "$WORK_DIR"/$TARGET_DIR

cd "$WORK_DIR"

echo Finished building gitmod version $VERSION packages for $DISTRO $DOCKER_TAG from committish $COMMITTISH
echo Packages coming out of the build are in $TARGET_DIR:

ls -l $TARGET_DIR

echo Thanks for using gitmod deb packager.
