# Task07 – LLVM Passes

## Overview

This task implements two LLVM function passes:

- Dead code elimination
- Runtime-based memory safety instrumentation

The dead code elimination pass removes redundant instructions, branches, basic blocks, and simple dead stores.

The memory safety pass instruments heap and stack operations to detect:

- Heap out-of-bounds accesses
- Heap use-after-free
- Stack out-of-bounds accesses
- Invalid or repeated calls to `free`

---

## Structure

```text
.
├── Makefile
├── README.md
├── dead-code-elimination/
│   ├── CMakeLists.txt
│   ├── DeadCodeElimination.cpp
│   └── Makefile
└── memory-safety/
    ├── CMakeLists.txt
    ├── Makefile
    ├── MemorySafety.cpp
    └── MemorySafetyRuntime.cpp
```

---

## Dead Code Elimination

The dead code elimination pass repeatedly applies local transformations until no further changes are found.

Implemented transformations:

- Removal of trivially dead instructions
- Simplification of conditional branches with identical successors
- Removal of empty forwarding basic blocks
- Elimination of simple dead stores to local stack allocations

Dead-store elimination is performed independently within each basic block and only for stack slots whose uses consist entirely of direct loads and stores.

The pass is registered under the pipeline name:

```text
dead-code-elimination
```

---

## Memory Safety

The memory safety pass instruments memory-management operations and memory accesses.

### Heap Instrumentation

Calls to `malloc` and `free` are replaced with runtime wrappers:

```text
malloc -> __runtime_malloc
free   -> __runtime_free
```

The runtime records heap allocations and retains metadata for freed blocks so that later accesses can be detected as use-after-free.

### Stack Instrumentation

Each LLVM `alloca` instruction is followed by a call to:

```text
__runtime_alloc
```

This registers the starting address and size of the stack object with the runtime.

### Memory Access Checks

Before every LLVM `load` and `store`, the pass inserts:

```text
__runtime_check_addr
```

The runtime verifies that the complete accessed range lies inside a registered block that has not been freed.

Tracked blocks are stored in an ordered map indexed by their starting addresses, allowing nearby allocations to be located in logarithmic time.

A 16-byte red zone around each registered block is used to detect nearby out-of-bounds accesses.

The pass is registered under the pipeline name:

```text
memory-safety
```

---

## Requirements

- Linux x86-64
- LLVM 16
- Clang 16
- CMake 3.18 or newer
- Ninja
- A C++17-compatible compiler

The default LLVM installation path is:

```text
/usr/lib/llvm-16
```

---

## Build

Build both passes:

```bash
make
```

Build only the dead code elimination pass:

```bash
make dce
```

Build only the memory safety pass and runtime:

```bash
make memory-safety
```

Remove all generated build files:

```bash
make clean
```

The generated libraries are located at:

```text
dead-code-elimination/build/libDeadCodeElimination.so
memory-safety/build/libMemorySafety.so
memory-safety/build/libMemorySafetyRuntime.a
```

---

## Usage

Compile a C source file to LLVM IR:

```bash
clang-16 -S -emit-llvm -O0 \
    -Xclang -disable-O0-optnone \
    input.c -o input.ll
```

### Dead Code Elimination

Run the dead code elimination pass:

```bash
opt-16 \
    -load-pass-plugin dead-code-elimination/build/libDeadCodeElimination.so \
    -passes=dead-code-elimination \
    -S input.ll -o optimized.ll
```

### Memory Safety

Instrument the LLVM IR:

```bash
opt-16 \
    -load-pass-plugin memory-safety/build/libMemorySafety.so \
    -passes=memory-safety \
    -S input.ll -o instrumented.ll
```

Link the instrumented program with the runtime library:

```bash
clang++-16 instrumented.ll \
    memory-safety/build/libMemorySafetyRuntime.a \
    -o instrumented
```

Run the resulting executable:

```bash
./instrumented
```

Invalid memory operations terminate the program and print:

```text
Illegal memory access
```

---

## Notes

- Both passes operate at function level.
- Relevant instructions are collected before the IR is modified.
- Runtime functions are excluded from memory-safety instrumentation.
- Object and access sizes are computed using LLVM's target data layout.
- The runtime uses an ordered allocation map rather than full shadow memory.
- The implementation focuses on the requirements of the practical assignment and is not intended to replace production tools such as AddressSanitizer.
