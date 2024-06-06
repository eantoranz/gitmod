#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -ex

# Script that will take care building/creating the package
yum install -y findutils rpmdevtools
cat "$REQUIREMENTS_FILE" | xargs yum install -y make git gcc

rpmdev-setuptree

# let's create the package every single time so we make sure we are _not_ working on a preexisting (and potentially busted!!!) file
git config --global --add safe.directory /mnt/work # so that git can work without complaining
git archive --format=tar.gz --prefix=gitmod-"$VERSION"/ -o gitmod-$VERSION.tar.gz $COMMITTISH
# tar zxvf gitmod-$VERSION.tar.gz -C ~/rpmbuild/SOURCES  --strip-components=1
cp gitmod-$VERSION.tar.gz ~/rpmbuild/SOURCES/gitmod

cp packages/rpm/gitmod.spec ~/rpmbuild/SPECS

rpmbuild --define "debug_package %{nil}" -ba packages/rpm/gitmod.spec || echo algo fallo en el rpmbuild

TARGET_DIR=packages/files/gitmod-$VERSION/$DISTRO-$DOCKER_TAG

if [ ! -d /mnt/work/$TARGET_DIR ]; then
	mkdir -p /mnt/work/$TARGET_DIR
fi

ARCH=$( uname -m )

cp -v /root/rpmbuild/RPMS/$ARCH/*.rpm /mnt/work/$TARGET_DIR

cd /mnt/work

echo Finished building packages for $DISTRO $DOCKER_TAG for gitmod version $VERSION from committish $COMMITTISH
echo Packages coming out of the build are in $TARGET_DIR:

ls -l $TARGET_DIR

echo Thanks for using gitmod rpm packager.
