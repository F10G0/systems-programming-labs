CXX ?= c++
CXXFLAGS ?= -g -Wall -O2 -std=c++17
CPPFLAGS += $(shell pkg-config --cflags protobuf)
LDLIBS += $(shell pkg-config --libs protobuf) -pthread

BUILD_DIR ?= build
GEN_DIR := $(BUILD_DIR)/generated
PROTO_FILE := proto/message.proto
PROTO_CC := $(GEN_DIR)/message.pb.cc
PROTO_H := $(GEN_DIR)/message.pb.h

LIBRARY := $(BUILD_DIR)/libutils.so
SERVER := $(BUILD_DIR)/server
CLIENT := $(BUILD_DIR)/client

.PHONY: all clean

all: $(LIBRARY) $(SERVER) $(CLIENT)

$(PROTO_CC) $(PROTO_H) &: $(PROTO_FILE) | $(GEN_DIR)
	protoc --proto_path=proto --cpp_out=$(GEN_DIR) $<

$(LIBRARY): src/utils.cpp include/utils.h $(PROTO_CC) $(PROTO_H) | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -Iinclude -I$(GEN_DIR) $(CXXFLAGS) -shared -fPIC \
		-o $@ src/utils.cpp $(PROTO_CC) $(LDLIBS)

$(SERVER): src/server.cpp include/utils.h $(LIBRARY) | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -Iinclude -I$(GEN_DIR) $(CXXFLAGS) \
		-o $@ src/server.cpp -L$(BUILD_DIR) -Wl,-rpath,'$$ORIGIN' -lutils $(LDLIBS)

$(CLIENT): src/client.cpp include/utils.h $(LIBRARY) | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) -Iinclude -I$(GEN_DIR) $(CXXFLAGS) \
		-o $@ src/client.cpp -L$(BUILD_DIR) -Wl,-rpath,'$$ORIGIN' -lutils $(LDLIBS)

$(GEN_DIR) $(BUILD_DIR):
	mkdir -p $@

clean:
	$(RM) -r $(BUILD_DIR)
