#!/bin/bash

# Copyright 2020 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

gcc -Wall gitfs.c include/gitfs.c `pkg-config fuse3 libgit2 --cflags --libs` -o gitfs

