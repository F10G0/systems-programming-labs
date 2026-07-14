# Systems Programming Labs (TUM)

This repository contains solutions for the **Systems Programming** practical course at TUM.

---

## Repository Structure

```text
.
├── .gitignore
├── Makefile
├── README.md
├── task00-sort/
├── task01-syscalls/
├── task02-fileio/
├── task03-processes/
├── task04-concurrency/
├── task05-memory/
├── task06-sockets/
└── task07-llvm/
```

---

## Task Matrix

| Task | Topic | Primary output | Key requirements |
| --- | --- | --- | --- |
| [Task 00](task00-sort/README.md) | External sorting | `task00-sort/build/sort` | C++17, POSIX |
| [Task 01](task01-syscalls/README.md) | System calls and tracing | Libraries and tracer under `task01-syscalls/build/` | Linux x86-64, `ptrace` |
| [Task 02](task02-fileio/README.md) | In-memory FUSE filesystem | `task02-fileio/build/memfs` | FUSE 2.x, `/dev/fuse` |
| [Task 03](task03-processes/README.md) | Shell and process management | `task03-processes/build/shell` | Flex, Bison, `libfl` |
| [Task 04](task04-concurrency/README.md) | Concurrent data structures | Libraries under `task04-concurrency/build/` | C11 atomics |
| [Task 05](task05-memory/README.md) | Custom allocator | `task05-memory/build/libmymalloc.so` | GNU11, pthreads, `sbrk()` |
| [Task 06](task06-sockets/README.md) | TCP and Protocol Buffers | Programs and library under `task06-sockets/build/` | C++17, Protobuf, `protoc` |
| [Task 07](task07-llvm/README.md) | LLVM passes | Plugins and runtime under `task07-llvm/build/` | LLVM/Clang 16, CMake, Ninja |

---

## Tasks

### task00-sort

- External sort implementation under memory constraints
- Supports ascending and reverse order (`-r`)

See [task00-sort](task00-sort/README.md).

---

### task01-syscalls

- System call wrappers using `syscall()`
- Direct syscall invocation via inline assembly
- Minimal syscall tracer using `ptrace()`

See [task01-syscalls](task01-syscalls/README.md).

---

### task02-fileio

- In-memory filesystem implemented using FUSE
- Supports hierarchical files and directories
- Read / write / append operations
- Symbolic links and filesystem statistics
- Log-based crash recovery (operation replay)

See [task02-fileio](task02-fileio/README.md).

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

See [task03-processes](task03-processes/README.md).

---

### task04-concurrency

- Spinlock implementation using C11 atomics
- Lock-based hashmap with per-bucket locking
- Lock-free hashmap using compare-and-swap (CAS)
- Logical deletion and physical unlinking

See [task04-concurrency](task04-concurrency/README.md).

---

### task05-memory

- Custom implementations of `malloc`, `calloc`, `realloc`, and `free`
- First-fit allocation with block splitting and coalescing
- Multiple cache-line-aligned arenas with per-arena locking
- Thread-local arena assignment
- Contention-sensitive heap reservation

See [task05-memory](task05-memory/README.md).

---

### task06-sockets

- Multithreaded TCP client-server application
- Protocol Buffers message serialization
- Length-prefixed message framing over TCP
- Fixed server worker pool with round-robin connection assignment
- I/O multiplexing using `poll()`
- Atomic shared counter with `ADD`, `SUB`, and `TERMINATION` operations

See [task06-sockets](task06-sockets/README.md).

---

### task07-llvm

- LLVM dead code elimination pass
- Removal of trivially dead instructions, redundant branches, and simple dead stores
- Runtime instrumentation for heap and stack memory accesses
- Detection of out-of-bounds accesses and use-after-free
- Ordered allocation tracking for efficient address lookup

See [task07-llvm](task07-llvm/README.md).

## Build

Build every assignment from the repository root:

```bash
make
```

Build one assignment by using its directory name as the target:


```bash
make task00-sort
```

The task Makefiles remain independently usable, for example `make -C task00-sort`.
All generated files are placed under the corresponding task's `build/` directory.
Build requirements differ by task; consult the task README before building.

Remove every generated build directory with:

```bash
make clean
```

---

## Usage

Refer to the linked task README for exact commands, arguments, outputs, and limitations.

---

## Notes

- Target platform: Linux x86-64
- Languages: C / C++
- Each task is independent and can be built separately.
- Source files live under `src/`, public headers under `include/`, protocol definitions under `proto/`, and supplementary material under `examples/` or `docs/` where applicable.
- Generated files are confined to task-local `build/` directories and are not tracked.
