# Task 06 — Socket Programming

## Overview

A multithreaded TCP client-server application implemented in C++.

The server maintains a shared counter and processes `ADD`, `SUB`, and `TERMINATION` requests from multiple concurrent clients. Client connections are distributed across a fixed pool of worker threads, with each worker using `poll()` to manage multiple sockets.

Messages are serialized using Protocol Buffers and transmitted with length-prefixed framing.

## Requirements

- Linux or another POSIX socket environment
- GNU Make and a C++17-compatible compiler
- POSIX threads
- Protocol Buffers runtime and development files
- `protoc` and `pkg-config`

## Repository Structure

```text
task06-sockets/
├── Makefile
├── README.md
├── include/
│   └── utils.h          # Public utility interface
├── proto/
│   └── message.proto    # Protocol Buffers message definition
└── src/
    ├── client.cpp       # Multithreaded TCP client
    ├── server.cpp       # Poll-based multithreaded server
    └── utils.cpp        # Socket and message utilities
```

## Build

Build all targets with:

```bash
make -C task06-sockets
```

This generates the Protocol Buffers sources and builds:

```text
task06-sockets/build/libutils.so
task06-sockets/build/server
task06-sockets/build/client
```

Generated Protocol Buffers sources are kept under `task06-sockets/build/generated/`.

## Usage

Start the server:

```bash
./build/server <num_threads> <port>
```

Example:

```bash
./build/server 4 8080
```

Start the client in another terminal:

```bash
./build/client <num_threads> <hostname> <port> <num_messages> <add> <sub>
```

Example:

```bash
./build/client 2 localhost 8080 10 5 3
```

Each client thread:

1. Establishes an independent TCP connection.
2. Sends alternating `ADD` and `SUB` operations, starting with `ADD`.
3. Sends a `TERMINATION` message.
4. Receives and prints the current counter value.

## Cleanup

```bash
make -C task06-sockets clean
```

## Limitations

- The server listens on the loopback interface only.
- Each worker processes complete messages synchronously for its assigned clients.
- The protocol does not provide authentication or encryption.
- Command-line numeric arguments are parsed without full validation.

## Troubleshooting

- Run `pkg-config --modversion protobuf` and `protoc --version` when generation or linking fails.
- The built executables use an origin-relative runtime library path so they can find `build/libutils.so` when launched from another working directory.
- Sandboxes may block local socket operations; the server uses loopback networking only.

## Notes

- The server listens on the loopback interface.
- Connections are assigned to workers in round-robin order.
- Each worker uses `poll()` to monitor its active clients.
- Newly accepted connections are transferred to workers through mutex-protected pending-client lists.
- The shared counter is implemented with `std::atomic<int64_t>`.
- Console output is protected by a mutex to prevent interleaving.
- Each network message consists of a 32-bit payload length followed by a serialized Protocol Buffers message.
- The server continues running until it is terminated externally.
