# Task01 – System Calls

## Overview

A small systems programming project exploring how user-space programs interact with the Linux kernel.

Includes:
- `read` / `write` wrappers using `syscall()`
- Direct system call invocation via inline assembly (x86-64)
- A minimal syscall tracer using `ptrace()`

---

## Structure

```
.
├── Makefile
├── syscall_wrapper.c
├── syscall_asm.c
└── syscall_tracer.c
```

---

## Build

```bash
make
```

---

## Usage

### Wrapper

```bash
LD_PRELOAD=./librw_wrapper.so cat file.txt
```

### Tracer

```bash
./tracer ls -l
```

---

## Notes

- Linux x86-64 only
- Demonstrates syscall ABI, errno handling, and ptrace-based tracing
