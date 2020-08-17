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

cache.o: src/gitmod/cache.c include/gitmod/cache.h
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

object.o: src/gitmod/object.c include/gitmod/object.h
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

root_tree.o: src/gitmod/root_tree.c include/gitmod/root_tree.h
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

thread.o: src/gitmod/thread.c include/gitmod/thread.h
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

lock.o: src/gitmod/lock.c include/gitmod/lock.h
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

gitmod.o: src/gitmod/gitmod.c include/gitmod.h lock.o root_tree.o thread.o object.o cache.o
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

gitmod: src/main.c gitmod.o
	$(CC) $< src/gitmod/*.o -o $@ $(CFLAGS)

test: tests/test.c tests/suites.h tests/suite*.c gitmod.o
	$(CC) $< tests/suite*.c src/gitmod/*.o -o $@ $(CFLAGSTEST)

clean:
	rm test gitmod src/gitmod/*.o
