# Task 04 — Concurrent Programming

## Overview

A systems programming project exploring synchronization and concurrent data structures.

Includes:
- A spinlock implemented using C11 atomics
- A lock-based hashmap using per-bucket spinlocks
- A lock-free hashmap using compare-and-swap (CAS)

## Requirements

- Linux or another platform with C11 atomic support
- GNU Make and a C11-compatible compiler
- POSIX-compatible shared-library support

---

## Repository Structure

```text
task04-concurrency/
├── Makefile
├── README.md
├── include/
│   ├── chashmap.h
│   └── cspinlock.h
└── src/
    ├── cspinlock.c
    ├── lockhashmap.c
    └── lockfreehashmap.c
```

---

## Build

```bash
make -C task04-concurrency
```

Generated outputs:

```text
task04-concurrency/build/libcspinlock.so
task04-concurrency/build/liblockhashmap.so
task04-concurrency/build/liblockfreehashmap.so
```

## Usage

Include `include/cspinlock.h` or `include/chashmap.h` and link exactly one implementation providing the relevant API. The hashmap operations return `0` on success/found and `1` on failure/not found, as documented in `include/chashmap.h`.

All concurrent users must stop before calling `free_hashmap` or freeing a spinlock.

## Cleanup

```bash
make -C task04-concurrency clean
```

## Limitations

- Destruction is not safe while other threads still access an object.
- The lock-free hashmap defers node reclamation until `free_hashmap`.
- `print_hashmap` is diagnostic output rather than a consistent global snapshot.

## Troubleshooting

- Confirm the compiler supports C11 atomics if `<stdatomic.h>` or atomic operations fail.
- Link only one hashmap implementation at a time because both libraries export the same API names.

---

## Notes

- Uses C11 atomic operations
- Spinlock implemented with `atomic_flag`
- Lock-based hashmap employs fine-grained bucket locking
- Lock-free hashmap uses CAS-based insertion and logical deletion
- Memory reclamation is deferred until `free_hashmap()`
