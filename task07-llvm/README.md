# Task 07 — LLVM Passes

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

## Repository Structure

```text
task07-llvm/
├── Makefile
├── README.md
├── dead-code-elimination/
│   ├── CMakeLists.txt
│   ├── Makefile
│   └── src/
│       └── DeadCodeElimination.cpp
└── memory-safety/
    ├── CMakeLists.txt
    ├── Makefile
    └── src/
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

## Build

Build both passes from the repository root:

```bash
make -C task07-llvm
```

Build only the dead code elimination pass:

```bash
make -C task07-llvm dce
```

Build only the memory safety pass and runtime:

```bash
make -C task07-llvm memory-safety
```

Remove all generated build files:

```bash
make -C task07-llvm clean
```

The generated libraries are located at:

```text
task07-llvm/build/dead-code-elimination/libDeadCodeElimination.so
task07-llvm/build/memory-safety/libMemorySafety.so
task07-llvm/build/memory-safety/libMemorySafetyRuntime.a
```

## Usage

The following commands assume `task07-llvm` is the working directory.

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
    -load-pass-plugin build/dead-code-elimination/libDeadCodeElimination.so \
    -passes=dead-code-elimination \
    -S input.ll -o optimized.ll
```

### Memory Safety

Instrument the LLVM IR:

```bash
opt-16 \
    -load-pass-plugin build/memory-safety/libMemorySafety.so \
    -passes=memory-safety \
    -S input.ll -o instrumented.ll
```

Link the instrumented program with the runtime library:

```bash
clang++-16 instrumented.ll \
    build/memory-safety/libMemorySafetyRuntime.a \
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

## Cleanup

```bash
make -C task07-llvm clean
```

## Limitations

- The passes focus on the assignment's local transformations rather than full LLVM optimization coverage.
- Heap instrumentation targets direct calls to `malloc` and `free`.
- The memory runtime uses a 16-byte red zone and a process-wide allocation map.
- The runtime is an educational implementation and is not a replacement for AddressSanitizer.

## Troubleshooting

- Confirm `clang-16`, `opt-16`, CMake, and Ninja are installed and that `/usr/lib/llvm-16` contains LLVM's CMake package.
- Missing optional zstd or CURL components do not prevent these plugin targets from building.

---

## Notes

- Both passes operate at function level.
- Relevant instructions are collected before the IR is modified.
- Runtime functions are excluded from memory-safety instrumentation.
- Object and access sizes are computed using LLVM's target data layout.
- The runtime uses an ordered allocation map rather than full shadow memory.
- The implementation focuses on the requirements of the practical assignment and is not intended to replace production tools such as AddressSanitizer.
