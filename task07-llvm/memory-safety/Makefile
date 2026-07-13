LLVM_DIR ?= /usr/lib/llvm-16
BUILD_DIR := build

RUNTIME_SOURCE := $(BUILD_DIR)/libMemorySafetyRuntimeCPP.a
RUNTIME_FINAL := $(BUILD_DIR)/libMemorySafetyRuntime.a

.PHONY: all cmake-build runtime clean

all: runtime

cmake-build:
	cmake -S . -B $(BUILD_DIR) \
		-G Ninja \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DLT_LLVM_INSTALL_DIR=$(LLVM_DIR)
	cmake --build $(BUILD_DIR)

runtime: cmake-build
	cp $(RUNTIME_SOURCE) $(RUNTIME_FINAL)

clean:
	rm -rf $(BUILD_DIR)
	