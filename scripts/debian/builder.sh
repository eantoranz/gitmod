#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -ex

# Script that will take care of doing the build process
apt-get update
if [ "$REQUIREMENTS_FILE" != "" ]; then
	cat "$REQUIREMENTS_FILE" | xargs apt-get install -y build-essential git
else
	apt-get install -y build-essential git
fi

make clean
make -j $( nproc )
make test
