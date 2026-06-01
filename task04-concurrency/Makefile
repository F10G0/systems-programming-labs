CC ?= cc
CFLAGS ?= -g -Wall -O2 -std=c11

.PHONY: all

all: libcspinlock.so liblockhashmap.so liblockfreehashmap.so
libcspinlock.so: cspinlock.c
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ $<

liblockhashmap.so: lockhashmap.c cspinlock.c
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ $^

liblockfreehashmap.so: lockfreehashmap.c
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ $<
