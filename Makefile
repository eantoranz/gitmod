# Copyright 2020 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

CC=gcc
CFLAGS=-Iinclude `pkg-config fuse3 libgit2 glib-2.0 --cflags --libs`
ifdef DEBUG
	CFLAGS+=-DGITMOD_DEBUG
endif
ifdef DEVELOPER
	CFLAGS+=-Wall -g
endif
CFLAGSTEST=$(CFLAGS) -lcunit -Itests

default: gitmod test

cache.o: src/cache.c include/cache.h
	$(CC) -c -o $@ $< $(CFLAGS)

object.o: src/object.c include/object.h
	$(CC) -c -o $@ $< $(CFLAGS)

root_tree.o: src/root_tree.c include/root_tree.h
	$(CC) -c -o $@ $< $(CFLAGS)

thread.o: src/thread.c include/thread.h
	$(CC) -c -o $@ $< $(CFLAGS)

lock.o: src/lock.c include/lock.h
	$(CC) -c -o $@ $< $(CFLAGS)

gitmod.o: src/gitmod.c include/gitmod.h lock.o root_tree.o thread.o object.o cache.o
	$(CC) -c -o $@ $< $(CFLAGS)

gitmod: src/main.c gitmod.o
	$(CC) $< *.o -o $@ $(CFLAGS)

test: tests/test.c tests/suites.h tests/suite*.c gitmod.o
	$(CC) $< tests/suite*.c *.o -o $@ $(CFLAGSTEST)

clean:
	rm test gitmod *.o
