# Task 03 — Shell Pipeline and Process Management

## Overview

A small Unix-like shell implementation focused on process creation, pipelines, redirection, and basic job control.

This project demonstrates how shells coordinate multiple processes using `fork`, `exec`, `pipe`, and `waitpid`.

---

## Features

- Command execution via `fork` + `execvp`
- Multi-stage pipelines using pipes
- Input / output redirection
- Foreground and background execution
- Builtin commands:
  - `exit`
  - `wait`
  - `kill`

## Requirements

- Linux or another environment providing the required POSIX process APIs
- GNU Make and a C compiler
- Flex, Bison, and the Flex runtime library (`libfl`)
- Parser-based command processing using Flex and Bison

---

## Architecture

The shell is structured into two main components:

- **parser**  
  Flex/Bison-based lexer and parser converting shell input into an internal pipeline representation.

- **execution layer**  
  Process management and pipeline execution implemented using Unix system calls.

---

## Repository Structure

```text
task03-processes/
├── Makefile
├── README.md
├── src/
│   ├── execute.c
│   └── main.c
└── parser/
│   ├── parse.h
│   ├── parse.y
│   └── scan.l
```

---

## Build

```bash
make -C task03-processes
```

The build generates parser sources under `task03-processes/build/generated/` and the executable `task03-processes/build/shell`.

## Usage

Run the shell:

```bash
./build/shell
```

Example commands:

```bash
ls -l
cat file.txt
grep hello file.txt
cat file.txt | wc -l
echo test > out.txt
cat < out.txt
sleep 10 &
```

Builtin commands:

```bash
wait
wait 1234

kill 1234

exit
exit 1
```

## Cleanup

```bash
make -C task03-processes clean
```

---

## Pipeline Execution

Pipelines are executed using:

- `pipe2`
- `fork`
- `dup2`
- `execvp`

Each command in the pipeline is executed in a separate child process.

Example:

```bash
cat file.txt | grep hello | wc -l
```

The shell connects stdout of one process to stdin of the next process through pipes.

---

## Redirection

Input and output redirection are supported:

```bash
grep hello < input.txt
echo test > output.txt
```

Implemented using:

- `open`
- `dup2`

---

## Background Execution

Commands ending with `&` are executed in the background:

```bash
sleep 100 &
```

Foreground pipelines block until all child processes terminate.  
Background pipelines immediately return control to the shell.

---

## Builtin Commands

### `exit [CODE]`

Terminate the shell with optional exit code.

### `wait [PID]`

Wait for:

- a specific child process (`wait PID`)
- any child process (`wait`)

### `kill PID`

Send `SIGTERM` to the specified process.

---

## Implementation Notes

The parser, lexer, shell framework, and main loop are based on the course-provided template.

Provided template files:

- `parser/parse.h`
- `parser/parse.y`
- `parser/scan.l`
- `src/main.c`

Implemented by me:

- `src/execute.c`
  - `run_pipeline()`
  - `run_builtin()`

---

## Limitations

- No support for:
  - quoting rules beyond the provided parser
  - environment variable expansion
  - job control (`fg`, `bg`)
  - signal forwarding
- The lexer supports only the documented restricted character and quoting rules.
- Background children are reaped only when the user invokes `wait`.

## Troubleshooting

- Install Flex, Bison, and `libfl` if parser generation or final linking fails.
- Use the documented limited syntax; unsupported characters may be discarded rather than diagnosed.

## Notes

- Target platform: Linux / POSIX systems
- Uses Unix process management primitives directly
- Focused on correctness and process control concepts rather than advanced shell features
