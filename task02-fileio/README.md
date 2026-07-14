# Task 02 — In-Memory FUSE Filesystem

## Overview

A simple **in-memory filesystem** implemented using FUSE.  
This project demonstrates how filesystem operations are mapped to internal data structures and how basic crash recovery can be implemented in user space.

---

## Features

- Hierarchical directories and files
- File read / write / append
- Symbolic links
- Filesystem statistics (`statfs`)
- Crash recovery via log replay
- Pure user-space filesystem (no kernel modification)

## Requirements

- Linux with FUSE support and access to `/dev/fuse`
- FUSE 2.x development headers and library (`FUSE_USE_VERSION 26`)
- GNU Make and a C compiler
- `fusermount` for unmounting

---

## Architecture

The filesystem is structured into three main components:

- **inode**  
  Tree-based structure representing files and directories.

- **log**  
  Append-only operation log used for crash recovery.  
  On startup, all operations are replayed to rebuild the filesystem state.

- **fuse**  
  Interface layer mapping system calls to in-memory operations.

---

## Repository Structure

```text
task02-fileio/
├── Makefile
├── README.md
├── src/        # Core implementation
│   ├── memfs.c
│   ├── inode.c
│   ├── inode.h
│   ├── log.c
│   └── log.h
└── examples/   # Annotated reference version
    └── memfs_single_annotated.c
```

---

## Build

```bash
make -C task02-fileio
```

The generated executable is `task02-fileio/build/memfs`.

## Usage

```bash
mkdir /tmp/mnt
./build/memfs /tmp/mnt
```

Example:

```bash
cd /tmp/mnt
mkdir dir
echo "hello" > file
cat file
```

Unmount:

```bash
fusermount -u /tmp/mnt
```

## Cleanup

Ensure the filesystem is unmounted before cleaning generated files:

```bash
make -C task02-fileio clean
```

---

## Assignment Requirements Coverage

This implementation satisfies all required features:

- Mountable filesystem (`./build/memfs [mount point]`)
- Flat directory structure (root-level files/directories)
- Hierarchical directory structure
- File read / write / append
- Symbolic links
- Correct file size reporting after writes
- Crash recovery (log replay on startup)
- Filesystem statistics (`statfs`)

---

## Implemented FUSE Operations

- `getattr`
- `read`, `write`
- `readdir`
- `mkdir`, `mknod`, `create`
- `open`
- `readlink`, `symlink`
- `statfs`

---

## Persistence

The filesystem uses a simple **log-based persistence mechanism**:

- Each modifying operation is appended to a log file
- On startup, the log is replayed
- The filesystem state is reconstructed in memory

This provides basic recovery without storing full snapshots. Large individual log records currently exceed the replay buffers and are listed as a known limitation below.

---

## Example (Annotated Version)

A fully annotated single-file implementation is provided:

```
examples/memfs_single_annotated.c
```

This version contains detailed comments explaining the design and is intended for learning and reference.

---

## Limitations

- The implementation is intended for small educational workloads rather than production use.
- Persistence uses a simple fixed-buffer text log.
- Paths and log records use fixed-size text fields.
- Removal and truncation operations are not implemented.

## Troubleshooting

- Verify `/dev/fuse` permissions when mounting fails with `Operation not permitted`.
- Install the FUSE 2.x development package if `fuse.h` or `-lfuse` is missing.
- Always unmount with `fusermount -u` before deleting or reusing the mount directory.

## Notes

- Maximum filename length: 255 characters
- File data can grow dynamically in memory, but persistence replay is currently limited by fixed log buffers
- No performance optimizations (focus on correctness and clarity)
