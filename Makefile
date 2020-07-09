# Copyright 2020 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

CC=gcc
CFLAGS=-g -Wall -Iinclude `pkg-config fuse3 libgit2 --cflags --libs`
CFLAGSTEST=-lcunit -Itests

default: gitmod test

root_tree.o: src/root_tree.c
	$(CC) -c -o root_tree.o $< $(CFLAGS)

root_tree_monitor.o: src/root_tree_monitor.c
	$(CC) -c -o root_tree_monitor.o $< $(CFLAGS)

lock.o: src/lock.c
	$(CC) -c -o lock.o $< $(CFLAGS)

gitmod.o: src/gitmod.c lock.o root_tree.o root_tree_monitor.o 
	$(CC) -c -o gitmod.o $< $(CFLAGS)

gitmod: src/main.c gitmod.o
	$(CC) $< *.o -o gitmod $(CFLAGS)

test: tests/test.c tests/suite*.c gitmod.o
	$(CC) $< tests/suite*.c *.o -o test $(CFLAGS) $(CFLAGSTEST)

clean:
	rm test gitmod *.o
