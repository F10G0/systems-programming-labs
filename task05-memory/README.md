# Task 05 — Memory Management

## Overview

A custom heap allocator implementing:

- `malloc`
- `calloc`
- `realloc`
- `free`

The allocator uses multiple arenas to reduce lock contention in multithreaded workloads. Threads are assigned to arenas using thread-local storage, while calls to `sbrk()` are protected by a global mutex.

## Requirements

- Linux x86-64 with a libc environment that provides `sbrk()`
- GNU Make and a GNU11-compatible C compiler
- POSIX threads and C11 atomics
- Dynamic-loader support for `LD_PRELOAD`

---

## Repository Structure

```text
task05-memory/
├── Makefile
├── README.md
└── src/
    └── allocator.c
```

---

## Implementation

The allocator provides:

- 8-byte aligned allocations
- First-fit free-block selection
- Block splitting and coalescing
- In-place `realloc` growth into an adjacent free block
- Multiple cache-line-aligned arenas
- Thread-local arena assignment
- Per-arena locking
- Contention-sensitive heap reservation
- Overflow checking for `calloc`

---

## Build

```bash
make -C task05-memory
```

This produces:

```text
task05-memory/build/libmymalloc.so
```

## Usage

Load the allocator in place of the system allocator using `LD_PRELOAD`:

```bash
LD_PRELOAD=./build/libmymalloc.so <program>
```

For example:

```bash
LD_PRELOAD=./build/libmymalloc.so ./test_program
```

Run the preload example from `task05-memory`.

## Cleanup

```bash
make -C task05-memory clean
```

## Limitations

- Alignment is 8 bytes, not necessarily the full alignment required of a general system `malloc` replacement.
- `sbrk()`-based allocation can conflict with other heap managers in the same process.
- The allocator is an educational implementation rather than a production libc replacement.

## Troubleshooting

- Ensure `LD_PRELOAD` points to an absolute or loader-resolvable path.
- Use preload behavior in a disposable process because the allocator can affect the loader and standard library before `main` runs.

---

## Notes

- Target platform: Linux x86-64
- Language: C11/GNU11
- Uses POSIX threads and C11 atomics
- Heap memory is obtained through `sbrk()`
