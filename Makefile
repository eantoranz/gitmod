# Copyright 2020 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

CC=gcc
CFLAGS=-g -Wall -Iinclude `pkg-config fuse3 libgit2 --cflags --libs`
CFLAGSTEST=-lcunit

default: gitmod test

gitmod.o:
	$(CC) -c -o gitmod.o src/gitmod.c $(CFLAGS)

gitmod: gitmod.o
	$(CC) src/main.c gitmod.o -o gitmod $(CFLAGS) 

test: gitmod.o
	$(CC) tests/test.c gitmod.o -o test $(CFLAGS) $(CFLAGSTEST)

clean:
	rm test gitmod gitmod.o
