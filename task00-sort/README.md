# task00-sort — External Sort Implementation

## Overview

This task implements a simplified version of Unix `sort(1)` with support for:

- Reading from standard input
- Sorting lines in ascending order
- Optional flag `-r` for descending order
- Handling large inputs under strict memory constraints

This implementation follows a standard external sorting pipeline:  
**run generation (chunking) + k-way merge**.

### Technical Focus

This implementation addresses sorting under memory constraints using an **external sorting strategy**:

1. **Chunking phase**: split input into memory-sized chunks and sort them individually
2. **Merge phase**: perform a k-way merge using a priority queue over temporary files

This approach is commonly used in real-world systems for large-scale data processing.

For detailed algorithmic explanation, see `sort.pdf`.

---

## Repository Structure

```text
task00-sort/
├── Makefile
├── README.md
├── sort.cpp
├── sort_annotated.cpp
└── sort.pdf
```

### Files

- **sort.cpp**  
  Main implementation using external sort (chunking + k-way merge)

- **sort_annotated.cpp**  
  Same implementation with detailed inline comments for learning purposes

- **sort.pdf**  
  Full explanation of the algorithm:
  - external sorting
  - memory constraints
  - k-way merge
  - complexity analysis

- **Makefile**  
  Build configuration

---

## Build

```bash
make
```

---

## Usage

Basic:

```bash
./sort < input.txt
```

Reverse order:

```bash
./sort -r < input.txt
```

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
