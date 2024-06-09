#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

#
# It will start an image to be able to build the project mounting the root of
# the project in /mnt/work
#
# Parameters
# - action (build, package, cli)
# - distro (fedora, centos, etc)
# - docker tag (7, 8, 41, etc)
# - git committish that will be used to build (needed only when calling build)

set -e

if [ $# -lt 3 ]; then
	echo Not enough parameters. Need to provide:
	echo - action "(build (b), package (p/pack), cli (c)"
	echo - distro "(fedora, centos, etc)"
	echo - distro docker tag "(41, 7, 8, etc, etc)"
	echo - if the action is \"package\", the committish to package.
	exit 1
fi

export ACTION=$1
export DISTRO=$2
export DOCKER_TAG=$3

echo action: $ACTION
echo distro: $DISTRO
echo distro docker tag: $DOCKER_TAG
case $ACTION in
b|build)
	ACTOR=builder
	ACTION=build
	;;
c|cli)
	ACTOR=cli
	ACTION=cli
	;;
p|pack|package)
	if [ $# -lt 4 ]; then
		echo You also need to specify which committish to package
		exit 1
	fi
	export COMMITTISH=$( git rev-parse --short $4 )
	export VERSION=$( git show $COMMITTISH:.gitmod_VERSION.txt 2> /dev/null | sed 's/.dirty//' )
	if [ "$VERSION" == "" ]; then
		echo Version file "(.gitmod_VERSION.txt)" does not exit in $COMMITTISH or it is empty. 1>&2
		exit 1
	fi
	echo git committish: $COMMITTISH
	echo git version: $VERSION
	ACTOR=packager
	ACTION=package
	;;
*)
	echo Unknown action $ACTION. Possible actions: build, package, cli.
	exit 1
esac


DOCKER_IMAGE="gitmod-rpmbuilder-$DISTRO-$DOCKER_TAG"

images=$( docker image list -q "$DOCKER_IMAGE" | wc -l )
if [ $images -eq 0 ]; then
	echo Image $DOCKER_IMAGE does not exist. Need to create it
	./scripts/docker/create-rpm-image.sh $DISTRO $DOCKER_TAG scripts/rpm/requirements.txt
fi

docker run --rm -ti -v "$PWD:/mnt/work" -w /mnt/work \
	-e DISTRO=$DISTRO \
	-e DOCKER_TAG=$DOCKER_TAG \
	-e VERSION=$VERSION \
	-e COMMITTISH=$COMMITTISH \
	--name gitmod-rpm$ACTOR-$DISTRO-$DOCKER_TAG \
	$DOCKER_IMAGE scripts/rpm/$ACTION.sh
