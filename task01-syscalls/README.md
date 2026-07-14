# Task 01 — System Calls

## Overview

A small systems programming project exploring how user-space programs interact with the Linux kernel.

Includes:
- `read` / `write` wrappers using `syscall()`
- Direct system call invocation via inline assembly (x86-64)
- A minimal syscall tracer using `ptrace()`

## Requirements

- Linux x86-64
- GNU Make and a C compiler
- Dynamic-loader support for `LD_PRELOAD`
- Permission to use `ptrace` for the tracer

---

## Repository Structure

```text
task01-syscalls/
├── Makefile
├── README.md
└── src/
    ├── syscall_wrapper.c
    ├── syscall_asm.c
    └── syscall_tracer.c
```

---

## Build

```bash
make -C task01-syscalls
```

Generated outputs:

```text
task01-syscalls/build/librw_wrapper.so
task01-syscalls/build/librw_asm.so
task01-syscalls/build/tracer
```

## Usage

### Wrapper

```bash
LD_PRELOAD=./build/librw_wrapper.so cat file.txt
```

### Tracer

```bash
./build/tracer ls -l
```

Run usage commands from `task01-syscalls`.

## Cleanup

```bash
make -C task01-syscalls clean
```

## Limitations

- The tracer reports only `read` and `write`.
- The tracer is Linux/x86-64-specific and depends on host `ptrace` policy.

## Troubleshooting

- `Operation not permitted` from the tracer normally means `ptrace` is blocked by sandbox, container, or kernel policy.
- Confirm the preload library path is correct if the loader cannot open the object.

---

## Notes

- Linux x86-64 only
- Demonstrates syscall ABI, errno handling, and ptrace-based tracing
