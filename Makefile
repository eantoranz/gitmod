# Copyright 2020 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

CC=gcc
CFLAGS=-g -Wall -Iinclude `pkg-config fuse3 libgit2 glib-2.0 --cflags --libs`
CFLAGSTEST=-lcunit -Itests

default: gitmod test

cache.o: src/cache.c
	$(CC) -c -o cache.o $< $(CFLAGS)

object.o: src/object.c
	$(CC) -c -o object.o $< $(CFLAGS)

root_tree.o: src/root_tree.c
	$(CC) -c -o root_tree.o $< $(CFLAGS)

thread.o: src/thread.c
	$(CC) -c -o thread.o $< $(CFLAGS)

lock.o: src/lock.c
	$(CC) -c -o lock.o $< $(CFLAGS)

gitmod.o: src/gitmod.c lock.o root_tree.o thread.o object.o cache.o
	$(CC) -c -o gitmod.o $< $(CFLAGS)

gitmod: src/main.c gitmod.o
	$(CC) $< *.o -o gitmod $(CFLAGS)

test: tests/test.c tests/suite*.c gitmod.o
	$(CC) $< tests/suite*.c *.o -o test $(CFLAGS) $(CFLAGSTEST)

clean:
	rm test gitmod *.o
