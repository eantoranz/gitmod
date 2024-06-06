#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2


# this script will be called from the root of the project that will be built.
#
# It will start an image to be able to build the project mounting the root of
# the project in /mnt/work
#
# Parameters
# - distro (debian, ubuntu)
# - ubuntu/debian tag to use for building (stable, testing, 22.04, 20.04, etc)

set -e

if [ $# -lt 2 ]; then
	echo Not enough parameters. Need to provide:
	echo - distro "(debian/ubuntu)"
	echo - distro docker tag "(stable, testing, buster, 22.04, 20.04, etc)"
	exit 1
fi

export DISTRO=$1
export DOCKER_TAG=$2

DOCKER_IMAGE="gitmod-debbuilder-$DISTRO-$DOCKER_TAG"

images=$( docker image list -q "$DOCKER_IMAGE" | wc -l )
if [ $images -eq 0 ]; then
	echo Image $DOCKER_IMAGE does not exist. Need to create it
	./scripts/docker/create-deb-image.sh $DISTRO $DOCKER_TAG scripts/deb/requirements.txt
fi

echo Building gitmod for $DISTRO $DOCKER_TAG

docker run --rm -ti -v "$PWD:/mnt/work" -w /mnt/work \
	-e DOCKER_TAG=$DOCKER_TAG \
	--name gitmod-debbuilder-$DISTRO-$DOCKER_TAG \
	$DOCKER_IMAGE scripts/deb/builder.sh
