CC ?= cc
CFLAGS ?= -g -Wall -O2 -std=c11
CPPFLAGS += -Iinclude
LDLIBS ?= -ldl

BUILD_DIR ?= build
SPINLOCK := $(BUILD_DIR)/libcspinlock.so
LOCKED_MAP := $(BUILD_DIR)/liblockhashmap.so
LOCKFREE_MAP := $(BUILD_DIR)/liblockfreehashmap.so

.PHONY: all clean

all: $(SPINLOCK) $(LOCKED_MAP) $(LOCKFREE_MAP)

$(SPINLOCK): src/cspinlock.c include/cspinlock.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared -fPIC -o $@ src/cspinlock.c $(LDLIBS)

$(LOCKED_MAP): src/lockhashmap.c src/cspinlock.c include/chashmap.h include/cspinlock.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared -fPIC -o $@ src/lockhashmap.c src/cspinlock.c $(LDLIBS)

$(LOCKFREE_MAP): src/lockfreehashmap.c include/chashmap.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared -fPIC -o $@ src/lockfreehashmap.c $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR)
