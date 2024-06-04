#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -ex

# Script that will take care of doing the build process
apt-get update
cat "$REQUIREMENTS_FILE" | xargs apt-get install -q -y build-essential git

make clean
make -j $( nproc )
make test
