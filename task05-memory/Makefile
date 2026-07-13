CC ?= cc
CFLAGS ?= -g -Wall -Wextra -O2 -std=gnu11 -fPIC -fno-builtin -pthread
LDFLAGS ?= -shared -pthread

TARGET := libmymalloc.so
SOURCE := allocator.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
