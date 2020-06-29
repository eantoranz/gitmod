# Copyright 2020 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

CC=gcc
CFLAGS=-g -Wall -Iinclude `pkg-config fuse3 libgit2 --cflags --libs`
CFLAGSTEST=-lcunit

default: gitfstrack test

gitfstrack.o:
	$(CC) -c -o gitfstrack.o include/gitfstrack.c $(CFLAGS)

gitfstrack: gitfstrack.o
	$(CC) main.c gitfstrack.o -o gitfstrack $(CFLAGS) 

test: gitfstrack.o
	$(CC) tests/test.c gitfstrack.o -o test $(CFLAGS) $(CFLAGSTEST)

clean:
	rm test gitfstrack gitfstrack.o
