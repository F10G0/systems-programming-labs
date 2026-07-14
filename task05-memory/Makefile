CC ?= cc
CFLAGS ?= -g -Wall -Wextra -O2 -std=gnu11 -fPIC -fno-builtin -pthread
LDFLAGS ?= -shared -pthread

BUILD_DIR ?= build
TARGET := $(BUILD_DIR)/libmymalloc.so
SOURCE := src/allocator.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR)
