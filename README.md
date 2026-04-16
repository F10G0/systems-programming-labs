# Systems Programming Labs (TUM)

This repository showcases a collection of low-level systems programming projects implemented in C/C++/Rust on Linux.

The projects focus on building efficient and robust system components under realistic constraints such as limited memory, concurrency, and large-scale data processing.

---

## Technical Scope

The repository covers key aspects of systems programming:

- File I/O and system calls  
- Concurrency and synchronization  
- Memory management  
- Process and thread management  
- Networking (client/server)  
- Performance profiling and optimization  

Each task emphasizes not only correctness, but also **resource management, scalability, and performance-aware design**.

---

## Repository Structure

Each task is organized as an independent module:

```
taskXX-name/
├── Makefile
├── README.md
├── name.cpp
├── name_annotated.cpp
└── name.pdf
```

- `*.cpp` — clean implementation  
- `*_annotated.cpp` — implementation with detailed explanations  
- `*.pdf` — full write-up including design, trade-offs, and complexity  
- `Makefile` — reproducible build configuration  

This structure is consistent across all tasks to ensure clarity, reproducibility, and ease of navigation.

---

## Design Philosophy

This repository is structured as a **systems programming portfolio**, focusing on:

- Clear separation between implementation and explanation  
- Reproducible builds using Makefiles  
- Emphasis on low-level reasoning and debugging  
- Handling real-world constraints (memory, I/O, concurrency)  
- Bridging algorithmic ideas with system-level constraints  

---

## Tasks

- Task 00 — External Sort (memory-constrained sorting with k-way merge)

More tasks will be added as the course progresses.

---

## Highlights

- Linux-first systems development  
- Low-level debugging and reasoning (gdb, printf)  
- Resource-aware and memory-safe implementations  
- Scalable solutions for large input sizes  
- Structured documentation and reproducible workflows
