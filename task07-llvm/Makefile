BUILD_DIR ?= build

.PHONY: all dce memory-safety clean

all: dce memory-safety

dce:
	$(MAKE) -C dead-code-elimination BUILD_DIR=../$(BUILD_DIR)/dead-code-elimination

memory-safety:
	$(MAKE) -C memory-safety BUILD_DIR=../$(BUILD_DIR)/memory-safety

clean:
	$(RM) -r $(BUILD_DIR)
