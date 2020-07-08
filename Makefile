# Copyright 2020 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

CC=gcc
CFLAGS=-g -Wall -Iinclude `pkg-config fuse3 libgit2 --cflags --libs`
CFLAGSTEST=-lcunit -Itests

default: gitmod test

root_tree.o:
	$(CC) -c -o root_tree.o src/root_tree.c $(CFLAGS)

root_tree_monitor.o:
	$(CC) -c -o root_tree_monitor.o src/root_tree_monitor.c $(CFLAGS)

lock.o:
	$(CC) -c -o lock.o src/lock.c $(CFLAGS)

gitmod.o: lock.o root_tree.o root_tree_monitor.o 
	$(CC) -c -o gitmod.o src/gitmod.c $(CFLAGS)

gitmod: gitmod.o
	$(CC) src/main.c *.o -o gitmod $(CFLAGS)

test: gitmod.o
	$(CC) tests/test.c tests/suite*.c *.o -o test $(CFLAGS) $(CFLAGSTEST)

clean:
	rm test gitmod *.o
