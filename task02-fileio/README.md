# Task02 - memfs (In-Memory Filesystem using FUSE)

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

```
.
├── src/        # Core implementation
│   ├── memfs.c
│   ├── inode.c / inode.h
│   ├── log.c / log.h
│
├── examples/   # Annotated reference version
│   └── memfs_single_annotated.c
│
├── Makefile
└── README.md
```

---

## Build

```bash
make
```

---

## Usage

```bash
mkdir /tmp/mnt
./memfs /tmp/mnt
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

---

## Assignment Requirements Coverage

This implementation satisfies all required features:

- Mountable filesystem (`./memfs [mount point]`)
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

This ensures recovery after crashes without storing full snapshots.

---

## Example (Annotated Version)

A fully annotated single-file implementation is provided:

```
examples/memfs_single_annotated.c
```

This version contains detailed comments explaining the design and is intended for learning and reference.

---

## Notes

- Maximum filename length: 255 characters
- Arbitrary file sizes supported
- No performance optimizations (focus on correctness and clarity)
