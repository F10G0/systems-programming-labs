# Task 00 — External Sort

## Overview

This task implements a simplified version of Unix `sort(1)` with support for:

- Reading from standard input
- Sorting lines in ascending order
- Optional flag `-r` for descending order
- Handling large inputs under strict memory constraints

This implementation follows a standard external sorting pipeline:  
**run generation (chunking) + k-way merge**.

## Design

This implementation addresses sorting under memory constraints using an **external sorting strategy**:

1. **Chunking phase**: split input into memory-sized chunks and sort them individually
2. **Merge phase**: perform a k-way merge using a priority queue over temporary files

This approach is commonly used in real-world systems for large-scale data processing.

## Requirements

- Linux or another POSIX-like environment
- GNU Make
- A C++17-compatible compiler
- Sufficient temporary-file space for inputs larger than memory

For a detailed algorithmic explanation, see `docs/sort.pdf`.

---

## Repository Structure

```text
task00-sort/
├── Makefile
├── README.md
├── src/
│   └── sort.cpp
├── examples/
│   └── sort_annotated.cpp
└── docs/
    └── sort.pdf
```

### Files

- **src/sort.cpp**: Main implementation using external sort (chunking + k-way merge)

- **examples/sort_annotated.cpp**: Same implementation with detailed inline comments for learning purposes

- **docs/sort.pdf**: Full explanation of the algorithm:
  - external sorting
  - memory constraints
  - k-way merge
  - complexity analysis

- **Makefile**  
  Build configuration

---

## Build

```bash
make -C task00-sort
```

The generated executable is `task00-sort/build/sort`.

## Usage

Basic:

```bash
./build/sort < input.txt
```

Reverse order:

```bash
./build/sort -r < input.txt
```

Run these commands from `task00-sort`. From the repository root, use `task00-sort/build/sort`.

## Cleanup

```bash
make -C task00-sort clean
```

## Limitations

- Memory accounting is approximate rather than allocator-accurate.
- Every temporary run remains open during the merge, so extremely large inputs may reach the process file-descriptor limit.
- Temporary-run I/O uses C-string operations and therefore does not preserve embedded NUL bytes.

## Troubleshooting

- A `tmpfile()` failure usually indicates insufficient temporary storage or an operating-system resource limit.
- A premature failure on very large input may require increasing the open-file limit or implementing multi-pass merging.

---

## Notes

- Input is read using `std::getline`
- Temporary runs are handled via `FILE*` and `tmpfile()`
- Memory usage is controlled via chunking
- Designed to work under ~128 MiB memory limit

---

## Highlights

- External sorting implementation under strict memory constraints
- Efficient k-way merge using `priority_queue`
- Scales beyond available memory via chunking and temporary runs
- Clean separation between implementation and algorithm documentation
