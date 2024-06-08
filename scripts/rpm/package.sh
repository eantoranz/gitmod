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
git archive --format=tar.gz --prefix=gitmod-"$VERSION"/ -o $TARGET_DIR/gitmod-$VERSION.tar.gz $COMMITTISH -- bin Makefile readme\* src
cp $TARGET_DIR/gitmod-$VERSION.tar.gz ~/rpmbuild/SOURCES/gitmod

cp packages/rpm/gitmod.spec ~/rpmbuild/SPECS

while true; do
  PACKAGE_ERROR=0
  rpmbuild --define "debug_package %{nil}" -ba packages/rpm/gitmod.spec || PACKAGE_ERROR=1
  if [ $PACKAGE_ERROR -eq 0 ]; then
    break
  fi
  echo Something failed creating deb package
  echo You can try any adjustemnt necessary to try to get the package to build successfully.
  echo If you run \"exit 0\", a new attempt to build the package will be carried out.
  echo If you run \"exit 1\", no more attempts will be tried and packaging process will be halted.
  /bin/bash || ( echo Halting packaging process; exit 1 )
done

ARCH=$( uname -m )

# making sure that the package can be installed
yum install -y /root/rpmbuild/RPMS/$ARCH/*.rpm && gitmod --help > /dev/null || ( echo gitmod rpm package installation test failed; exit 1 )
echo Package can be installed without issues, binary is present in \$PATH

cp -v /root/rpmbuild/RPMS/$ARCH/*.rpm "$WORK_DIR"/$TARGET_DIR

cd "$WORK_DIR"

echo Finished building gitmod version $VERSION packages for $DISTRO $DOCKER_TAG from committish $COMMITTISH
echo Packages coming out of the build are in $TARGET_DIR:

ls -l $TARGET_DIR

echo Thanks for using gitmod rpm packager.
