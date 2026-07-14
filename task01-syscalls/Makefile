CC ?= cc
CFLAGS ?= -g -Wall -O2

BUILD_DIR ?= build
WRAPPER := $(BUILD_DIR)/librw_wrapper.so
ASM_WRAPPER := $(BUILD_DIR)/librw_asm.so
TRACER := $(BUILD_DIR)/tracer

.PHONY: all clean

all: $(WRAPPER) $(ASM_WRAPPER) $(TRACER)

$(WRAPPER): src/syscall_wrapper.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

$(ASM_WRAPPER): src/syscall_asm.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

$(TRACER): src/syscall_tracer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR)
