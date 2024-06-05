#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -ex

# Script that will take care of doing the build process
yum install -y findutils # provides xargs
cat "$REQUIREMENTS_FILE" | xargs yum install -y make git gcc

make clean
make -j $( nproc )
make test
