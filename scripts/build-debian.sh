#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2


# this script will be called from the root of the project that will be built.
#
# It will start an image to be able to build the project mounting the root of
# the project in /mnt/work
#
# Parameters
# - debian tag to use for building

if [ $# -lt 1 ]; then
	echo Not enough parameters. Need to provide:
	echo - debian docker tag "(stable, testing, buster, etc)"
	exit 1
fi

GITMOD_ROOT="$( git rev-parse --show-toplevel )"

export DEBIAN_TAG=$1

echo Building for debian $DEBIAN_TAG

docker pull debian:$DEBIAN_TAG

docker run --rm -ti -v "$PWD:/mnt/work" -w /mnt/work \
	-e REQUIREMENTS_FILE=scripts/debian/requirements.txt \
	--name gitmod-debian-builder-$DEBIAN_TAG \
	debian:$DEBIAN_TAG scripts/debian/builder.sh 
