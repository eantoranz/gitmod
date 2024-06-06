#!/bin/bash

# Copyright 2024 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

set -ex

make clean
make -j $( nproc )
make -j $( nproc ) test
