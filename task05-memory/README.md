# Task05 – Memory Management

## Overview

A custom heap allocator implementing:

- `malloc`
- `calloc`
- `realloc`
- `free`

The allocator uses multiple arenas to reduce lock contention in multithreaded workloads. Threads are assigned to arenas using thread-local storage, while calls to `sbrk()` are protected by a global mutex.

---

## Structure

```text
.
├── Makefile
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
make
```

This produces:

```text
libmymalloc.so
```

---

## Usage

Load the allocator in place of the system allocator using `LD_PRELOAD`:

```bash
LD_PRELOAD=./libmymalloc.so <program>
```

For example:

```bash
LD_PRELOAD=./libmymalloc.so ./test_program
```

---

## Notes

- Target platform: Linux x86-64
- Language: C11/GNU11
- Uses POSIX threads and C11 atomics
- Heap memory is obtained through `sbrk()`
