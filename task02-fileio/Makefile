CC ?= cc
CFLAGS ?= -g -Wall -O2 -D_FILE_OFFSET_BITS=64
LDLIBS ?= -lfuse

BUILD_DIR ?= build
SOURCES := src/memfs.c src/inode.c src/log.c
TARGET := $(BUILD_DIR)/memfs

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR)
