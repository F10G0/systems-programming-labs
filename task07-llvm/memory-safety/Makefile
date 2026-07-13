BUILD_DIR := build

RUNTIME_SOURCE := $(BUILD_DIR)/libMemorySafetyRuntimeCPP.a
RUNTIME_FINAL := $(BUILD_DIR)/libMemorySafetyRuntime.a

.PHONY: all cmake-build runtime clean

all: runtime

cmake-build:
	cmake -S . -B $(BUILD_DIR) \
		-G Ninja \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo
	cmake --build $(BUILD_DIR)

runtime: cmake-build
	cp $(RUNTIME_SOURCE) $(RUNTIME_FINAL)

clean:
	rm -rf $(BUILD_DIR)
	