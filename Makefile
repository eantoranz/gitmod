# Copyright 2020 Edmundo Carmona Antoranz
# Released under the terms of GPLv2

CC=gcc
CFLAGS=-Isrc/include `pkg-config fuse3 libgit2 glib-2.0 --cflags --libs`
ifdef DEBUG
	CFLAGS+=-DGITMOD_DEBUG
endif
ifdef DEVELOPER
	CFLAGS+=-Wall -g
endif
CFLAGSTEST=$(CFLAGS) -lcunit -Itests

default: gitmod

cache.o: src/gitmod/cache.c src/include/gitmod/cache.h
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

object.o: src/gitmod/object.c src/include/gitmod/object.h
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

root_tree.o: src/gitmod/root_tree.c src/include/gitmod/root_tree.h
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

thread.o: src/gitmod/thread.c src/include/gitmod/thread.h
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

lock.o: src/gitmod/lock.c src/include/gitmod/lock.h
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

gitmod.o: src/gitmod/gitmod.c src/include/gitmod.h lock.o root_tree.o thread.o object.o cache.o
	$(CC) -c -o src/gitmod/$@ $< $(CFLAGS)

gitmod: src/gitmod/main.c gitmod.o
	$(CC) $< src/gitmod/*.o -o bin/$@ $(CFLAGS)

unit_tests: src/tests/unit_tests/main.c src/tests/unit_tests/suites.h src/tests/unit_tests/suite*.c gitmod.o
	$(CC) $< src/tests/unit_tests/suite*.c src/gitmod/*.o -o bin/$@ $(CFLAGSTEST)

install:
	mkdir -p $(DESTDIR)$(prefix)/bin
	install bin/gitmod $(DESTDIR)$(prefix)/bin

clean:
	rm -f bin/unit_tests bin/gitmod src/gitmod/*.o

format:
	indent -l120 -linux src/gitmod/*.c src/include/*.h src/include/gitmod/*.h src/tests/unit_tests/*.c src/tests/unit_tests/*.h
	find ./ -name '*~' -delete
