# Set your preferred compilers and flags
CC ?= cc
CFLAGS ?= -g -Wall -O2
CXX ?= c++
CXXFLAGS ?= -g -Wall -O2 -std=c++17
CARGO ?= cargo
RUSTFLAGS ?= -g

# Build target
all:
	$(CXX) $(CXXFLAGS) -o sort sort.cpp

# Run tests
check: all
	$(MAKE) -C tests check

# Optional clean
clean:
	rm -f sort
