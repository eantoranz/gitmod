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

if [ $# -lt 2 ]; then
	echo Not enough parameters. Need to provide:
	echo - distro "(debian/ubuntu)"
	echo - debian/ubuntu docker tag "(stable, testing, buster, 22.04, 20.04, etc)"
	exit 1
fi

export DISTRO=$1
export DOCKER_TAG=$2

echo Building for $DISTRO $DOCKER_TAG

docker pull debian:$DEBIAN_TAG

docker run --rm -ti -v "$PWD:/mnt/work" -w /mnt/work \
	-e REQUIREMENTS_FILE=scripts/deb/requirements.txt \
	-e DOCKER_TAG=$DOCKER_TAG \
	--name gitmod-builder-$DISTRO-$DOCKER_TAG \
	$DISTRO:$DOCKER_TAG scripts/deb/builder.sh
