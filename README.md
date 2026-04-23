# Systems Programming Labs (TUM)

This repository contains solutions for the **Systems Programming** practical course at TUM.

---

## Repository Structure

```text
.
├── README.md
├── task00-sort
└── task01-system-calls
```

---

## Tasks

### task00-sort

- External sort implementation under memory constraints
- Supports ascending and reverse order (`-r`)

➡️ See: `task00-sort/`

---

### task01-system-calls

- System call wrappers using `syscall()`
- Direct syscall invocation via inline assembly
- Minimal syscall tracer using `ptrace()`

➡️ See: `task01-system-calls/`

---

## Build

Each task is self-contained.

Example:

```bash
cd task00-sort
make
```

---

## Usage

Refer to the README inside each task directory for details.

---

## Notes

- Target platform: Linux x86-64
- Languages: C / C++
- Each task is independent and can be built separately
