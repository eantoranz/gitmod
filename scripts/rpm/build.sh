#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -e

make clean
make -j $( nproc )
make -j $( nproc ) unit_tests
