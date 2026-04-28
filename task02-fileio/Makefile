
CC ?= cc
CFLAGS ?= -g -Wall -O2 -D_FILE_OFFSET_BITS=64
SRC = src/memfs.c src/inode.c src/log.c
LDFLAGS = -lfuse

.PHONY: all

all:
	$(CC) $(CFLAGS) -o memfs $(SRC) $(LDFLAGS)
