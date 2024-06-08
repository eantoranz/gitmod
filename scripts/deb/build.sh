#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -e

# Script that will take care of doing the build process
make clean
make -j $( nproc )
make -j $( nproc ) unit_tests
