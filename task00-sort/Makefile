CXX ?= c++
CXXFLAGS ?= -g -Wall -O2 -std=c++17

BUILD_DIR ?= build
TARGET := $(BUILD_DIR)/sort
SOURCE := src/sort.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR)
