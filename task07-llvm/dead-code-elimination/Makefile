LLVM_DIR ?= /usr/lib/llvm-16
BUILD_DIR ?= ../build/dead-code-elimination

.PHONY: all clean

all:
	cmake -S . -B $(BUILD_DIR) \
		-G Ninja \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DLT_LLVM_INSTALL_DIR=$(LLVM_DIR)
	cmake --build $(BUILD_DIR)

clean:
	$(RM) -r $(BUILD_DIR)
