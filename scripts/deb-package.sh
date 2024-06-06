#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

# this script will be called from the root of the project that will be built.
#
# It will start an image to be able to build the project mounting the root of
# the project in /mnt/work
#
# Parameters
# - distro (ubuntu, debian)
# - docker tag (stable, testing, buster, 22.04, etc)
# - git committish that will be used to build

if [ $# -lt 3 ]; then
	echo Not enough parameters. Need to provide:
	echo - distro "(ubuntu, debian)"
	echo - distro docker tag "(stable, testing, buster, 22.04, etc)"
	echo - git committish that will be used for building/packaging
	exit 1
fi

export DISTRO=$1
export DOCKER_TAG=$2
export COMMITTISH=$3
export VERSION=$( git show $COMMITTISH:.gitmod_VERSION.txt 2> /dev/null | sed 's/.dirty//' )

if [ "$VERSION" == "" ]; then
	echo Version file "(.gitmod_VERSION.txt)" does not exit in $COMMITTISH or it is empty 1>&2
	exit 1
fi

echo distro: $DISTRO
echo distro docker tag: $DOCKER_TAG
echo git committish: $COMMITTISH
echo gitmod version: $VERSION

DOCKER_IMAGE="gitmod-debbuilder-$DISTRO-$DOCKER_TAG"

images=$( docker image list -q "$DOCKER_IMAGE" | wc -l )
if [ $images -eq 0 ]; then
	echo Image $DOCKER_IMAGE does not exist. Need to create it
	./scripts/docker/create-deb-image.sh $DISTRO $DOCKER_TAG scripts/deb/requirements.txt
fi

docker run --rm -ti -v "$PWD:/mnt/work" -w /mnt/work \
	-e DISTRO=$DISTRO \
	-e DOCKER_TAG=$DOCKER_TAG \
	-e VERSION=$VERSION \
	-e COMMITTISH=$COMMITTISH \
	--name gitmod-debpackager-$DISTRO-$DOCKER_TAG \
	$DOCKER_IMAGE scripts/deb/packager.sh
