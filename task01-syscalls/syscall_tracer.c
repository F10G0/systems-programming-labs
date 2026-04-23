#define _GNU_SOURCE
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

// Minimal syscall tracer (subset of strace)
// Only traces read/write system calls.
int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    pid_t pid = fork();
    if (pid == -1) return 1;

    // Child: request tracing and execute target program
    if (!pid) {
        ptrace(PTRACE_TRACEME);
        execvp(argv[1], &argv[1]);
        return 1; // exec failed
    }

    int status;
    struct user_regs_struct regs;

    // Main tracing loop
    do {
        // Run until next syscall entry/exit
        ptrace(PTRACE_SYSCALL, pid, NULL, 0);
        waitpid(pid, &status, 0);
        // Read registers at syscall entry
        ptrace(PTRACE_GETREGS, pid, NULL, &regs);

        unsigned long SYS_id = regs.orig_rax;
        // Only handle read/write, skip others
        if (SYS_id != SYS_read && SYS_id != SYS_write) continue;
        // Print syscall name and arguments (entry point)
        fprintf(stderr, "%s(%llu, 0x%llx, %llu) = ", SYS_id == SYS_read ? "read" : "write", regs.rdi, regs.rsi, regs.rdx);

        // Advance to syscall exit
        ptrace(PTRACE_SYSCALL, pid, NULL, 0);
        waitpid(pid, &status, 0);
        // Read return value
        ptrace(PTRACE_GETREGS, pid, NULL, &regs);

        // Print result
        fprintf(stderr, "%lld\n", regs.rax);
    } while (!WIFEXITED(status));

    return 0;
}
