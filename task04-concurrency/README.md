# Task04 – Concurrent Programming

## Overview

A systems programming project exploring synchronization and concurrent data structures.

Includes:
- A spinlock implemented using C11 atomics
- A lock-based hashmap using per-bucket spinlocks
- A lock-free hashmap using compare-and-swap (CAS)

---

## Structure

```text
.
├── Makefile
├── cspinlock.h
├── cspinlock.c
├── chashmap.h
├── lockhashmap.c
└── lockfreehashmap.c
```

---

## Build

```bash
make
```

---

## Notes

- Uses C11 atomic operations
- Spinlock implemented with `atomic_flag`
- Lock-based hashmap employs fine-grained bucket locking
- Lock-free hashmap uses CAS-based insertion and logical deletion
- Memory reclamation is deferred until `free_hashmap()`
