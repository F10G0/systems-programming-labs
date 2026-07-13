# Task06 - Socket Programming

## Overview

A multithreaded TCP client-server application implemented in C++.

The server maintains a shared counter and processes `ADD`, `SUB`, and `TERMINATION` requests from multiple concurrent clients. Client connections are distributed across a fixed pool of worker threads, with each worker using `poll()` to manage multiple sockets.

Messages are serialized using Protocol Buffers and transmitted with length-prefixed framing.

## Structure

```text
.
├── client.cpp       # Multithreaded TCP client
├── server.cpp       # Poll-based multithreaded server
├── utils.cpp        # Socket and message utilities
├── utils.h          # Public utility interface
├── message.proto    # Protocol Buffers message definition
├── Makefile         # Build configuration
└── README.md
```

## Build

The project requires:

- A C++17-compatible compiler
- POSIX threads
- Protocol Buffers
- `protoc`
- `pkg-config`

Build all targets with:

```bash
make
```

This generates the Protocol Buffers sources and builds:

```text
libutils.so
server
client
```

Remove generated files and binaries with:

```bash
make clean
```

## Usage

Start the server:

```bash
./server <num_threads> <port>
```

Example:

```bash
./server 4 8080
```

Start the client in another terminal:

```bash
./client <num_threads> <hostname> <port> <num_messages> <add> <sub>
```

Example:

```bash
./client 2 localhost 8080 10 5 3
```

Each client thread:

1. Establishes an independent TCP connection.
2. Sends alternating `ADD` and `SUB` operations, starting with `ADD`.
3. Sends a `TERMINATION` message.
4. Receives and prints the current counter value.

## Notes

- The server listens on the loopback interface.
- Connections are assigned to workers in round-robin order.
- Each worker uses `poll()` to monitor its active clients.
- Newly accepted connections are transferred to workers through mutex-protected pending-client lists.
- The shared counter is implemented with `std::atomic<int64_t>`.
- Console output is protected by a mutex to prevent interleaving.
- Each network message consists of a 32-bit payload length followed by a serialized Protocol Buffers message.
- The server continues running until it is terminated externally.
