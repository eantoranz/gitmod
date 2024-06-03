#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

# this script will be called from the root of the project that will be built.
#
# It will start an image to be able to build the project mounting the root of
# the project in /mnt/work
#
# Parameters
# - git committish that will be used to build
#
# environment variables:
# - DEBIAN_TAG: debian docker tag used for building (default: stable)

function get_gitmod_version {
	VERSION=$( git show $1:.gitmod_VERSION.txt 2> /dev/null | sed 's/.dirty//' )
	if [ "$VERSION" == "" ]; then
		echo Version file "(.gitmod_VERSION.txt)" does not exit in $1 or it is empty 1>&2
		exit 1
	fi
	echo $VERSION
}

if [ $# -lt 2 ]; then
	echo Not enough parameters. Need to provide:
	echo - debian docker tag "(stable, testing, buster, etc)"
	echo - git committish that will be used for building/packaging
	exit 1
fi

GITMOD_ROOT="$( git rev-parse --show-toplevel )"

export DEBIAN_TAG=$1
export COMMITTISH=$2
export VERSION=$( get_gitmod_version $COMMITTISH )

if [ "$VERSION" == "" ]; then
	exit 1
fi

echo debian docker tag: $DEBIAN_TAG
echo git committish: $COMMITTISH
echo gitmod version: $VERSION

echo Pulling debian:$DEBIAN_TAG docker image
docker pull debian:$DEBIAN_TAG

docker run --rm -ti -v "$PWD:/mnt/work" -w /mnt/work \
	-e DEBIAN_TAG=$DEBIAN_TAG \
	-e VERSION=$VERSION \
	-e COMMITTISH=$COMMITTISH \
	-e REQUIREMENTS_FILE=scripts/debian/requirements.txt \
	--name gitmod-debian-builder-$DEBIAN_TAG \
	debian:$DEBIAN_TAG scripts/debian/packager.sh
