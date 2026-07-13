# Systems Programming Labs (TUM)

This repository contains solutions for the **Systems Programming** practical course at TUM.

---

## Repository Structure

```text
.
├── README.md
├── task00-sort
├── task01-syscalls
├── task02-fileio
├── task03-processes
├── task04-concurrency
├── task05-memory
├── task06-sockets
└── task07-llvm
```

---

## Tasks

### task00-sort

- External sort implementation under memory constraints
- Supports ascending and reverse order (`-r`)

➡️ See: `task00-sort/`

---

### task01-syscalls

- System call wrappers using `syscall()`
- Direct syscall invocation via inline assembly
- Minimal syscall tracer using `ptrace()`

➡️ See: `task01-syscalls/`

---

### task02-fileio

- In-memory filesystem implemented using FUSE
- Supports hierarchical files and directories
- Read / write / append operations
- Symbolic links and filesystem statistics
- Log-based crash recovery (operation replay)

➡️ See: `task02-fileio/`

---

### task03-processes

- Unix-like shell implementation
- Multi-stage pipeline execution using `pipe`
- Input / output redirection
- Background process execution
- Builtin process management commands:
  - `wait`
  - `kill`
  - `exit`
- Flex/Bison-based command parser

➡️ See: `task03-processes/`

---

### task04-concurrency

- Spinlock implementation using C11 atomics
- Lock-based hashmap with per-bucket locking
- Lock-free hashmap using compare-and-swap (CAS)
- Logical deletion and physical unlinking

➡️ See: `task04-concurrency/`

---

### task05-memory

- Custom implementations of `malloc`, `calloc`, `realloc`, and `free`
- First-fit allocation with block splitting and coalescing
- Multiple cache-line-aligned arenas with per-arena locking
- Thread-local arena assignment
- Contention-sensitive heap reservation

➡️ See: `task05-memory/`

---

### task06-sockets

- Multithreaded TCP client-server application
- Protocol Buffers message serialization
- Length-prefixed message framing over TCP
- Fixed server worker pool with round-robin connection assignment
- I/O multiplexing using `poll()`
- Atomic shared counter with `ADD`, `SUB`, and `TERMINATION` operations

➡️ See: `task06-sockets/`

---

### task07-llvm

- LLVM dead code elimination pass
- Removal of trivially dead instructions, redundant branches, and simple dead stores
- Runtime instrumentation for heap and stack memory accesses
- Detection of out-of-bounds accesses and use-after-free
- Ordered allocation tracking for efficient address lookup

➡️ See: `task07-llvm/`

## Build

Each task is self-contained.

Example:

```bash
cd task00-sort
make
```

---

## Usage

Refer to the README inside each task directory for details.

---

## Notes

- Target platform: Linux x86-64
- Languages: C / C++
- Each task is independent and can be built separately
