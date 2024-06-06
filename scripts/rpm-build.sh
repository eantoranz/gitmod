#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2


# this script will be called from the root of the project that will be built.
#
# It will start an image to be able to build the project mounting the root of
# the project in /mnt/work
#
# Parameters
# - distro (centos, fedorea)
# - distro docker tag to use for building (9.0, 8.0, etc)

if [ $# -lt 2 ]; then
	echo Not enough parameters. Need to provide:
	echo - distro "(centos, fedora)"
	echo - distro docker tag "(9.0, 8.0, etc)"
	exit 1
fi

export DISTRO=$1
export DOCKER_TAG=$2

echo Building for $DISTRO $DOCKER_TAG

docker pull $DISTRO:$DEBIAN_TAG

docker run --rm -ti -v "$PWD:/mnt/work" -w /mnt/work \
	-e REQUIREMENTS_FILE=scripts/rpm/requirements.txt \
	-e DOCKER_TAG=$DOCKER_TAG \
	--name gitmod-rpmbuilder-$DISTRO-$DOCKER_TAG \
	$DISTRO:$DOCKER_TAG scripts/rpm/builder.sh
