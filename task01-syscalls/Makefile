CC ?= cc
CFLAGS ?= -g -Wall -O2

.PHONY: all

all: librw_wrapper.so librw_asm.so tracer
librw_wrapper.so: syscall_wrapper.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

librw_asm.so: syscall_asm.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

tracer: syscall_tracer.c
	$(CC) $(CFLAGS) -o $@ $<
