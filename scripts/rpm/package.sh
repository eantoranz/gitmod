#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -e

rpmdev-setuptree

# Script that will take care building/creating the package

WORK_DIR="$PWD"

TARGET_DIR=packages/files/gitmod-$VERSION

if [ ! -d $TARGET_DIR ]; then
	mkdir -p $TARGET_DIR
fi

# let's create the package every single time so we make sure we are _not_ working on a preexisting (and potentially busted!!!) file
git config --global --add safe.directory "$WORK_DIR" # so that git can work without complaining
git archive --format=tar.gz --prefix=gitmod-"$VERSION"/ -o $TARGET_DIR/gitmod-$VERSION.tar.gz $COMMITTISH -- Makefile src readme\*
cp $TARGET_DIR/gitmod-$VERSION.tar.gz ~/rpmbuild/SOURCES/gitmod

cp packages/rpm/gitmod.spec ~/rpmbuild/SPECS

rpmbuild --define "debug_package %{nil}" -ba packages/rpm/gitmod.spec

ARCH=$( uname -m )

# making sure that the package can be installed
yum install -y /root/rpmbuild/RPMS/$ARCH/*.rpm || ( echo gitmod test package installation failed; exit 1 )
echo Package can be installed without issues

cp -v /root/rpmbuild/RPMS/$ARCH/*.rpm "$WORK_DIR"/$TARGET_DIR

cd "$WORK_DIR"

echo Finished building gitmod version $VERSION packages for $DISTRO $DOCKER_TAG from committish $COMMITTISH
echo Packages coming out of the build are in $TARGET_DIR:

ls -l $TARGET_DIR

echo Thanks for using gitmod rpm packager.
