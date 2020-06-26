#!/bin/bash

# Copyright 2020 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

gcc -Wall main.c include/gitfstrack.c `pkg-config fuse3 libgit2 --cflags --libs` -o gitfstrack -g && \
gcc -Wall tests/test.c include/gitfstrack.c `pkg-config libgit2 --cflags --libs` -lcunit -o test -g

