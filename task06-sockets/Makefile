CXX ?= c++
CXXFLAGS ?= -g -Wall -O2 -std=c++17
PROTOBUF_FLAGS := $(shell pkg-config --cflags --libs protobuf)
LDLIBS := $(PROTOBUF_FLAGS) -lpthread

.PHONY: all clean

all: libutils.so server client

message.pb.cc: message.proto
	protoc --cpp_out=. $<

libutils.so: utils.cpp utils.h message.pb.cc
	$(CXX) $(CXXFLAGS) -shared -fPIC -o $@ utils.cpp message.pb.cc $(LDLIBS)

server: server.cpp utils.h libutils.so
	$(CXX) $(CXXFLAGS) -o $@ server.cpp -L. -Wl,-rpath=. -lutils $(LDLIBS)

client: client.cpp utils.h libutils.so
	$(CXX) $(CXXFLAGS) -o $@ client.cpp -L. -Wl,-rpath=. -lutils $(LDLIBS)

clean:
	$(RM) server client libutils.so message.pb.cc message.pb.h
	