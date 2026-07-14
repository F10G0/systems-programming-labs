#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

// Direct syscall invocation using x86-64 ABI and inline assembly.
// This bypasses libc and manually issues the `syscall` instruction.
static ssize_t my_syscall(long rax, long rdi, long rsi, long rdx) {
    long ret;

    // Registers:
    // rax = syscall number
    // rdi, rsi, rdx = first three arguments
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(rax), "D"(rdi), "S"(rsi), "d"(rdx)
        : "rcx", "r11", "memory"   // clobbered by syscall
    );

    // Linux returns errors in range [-4095, -1]
    // Convert to libc-style: return -1 and set errno
    if ((unsigned long)ret >= (unsigned long)-4095) {
        errno = (int)-ret;
        return -1;
    }

    return ret;
}

ssize_t read(int fd, void *buf, size_t count) {
    if (!count) return 0;
    // Cast pointer to integer for register passing
    return my_syscall(SYS_read, fd, (long)buf, (long)count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    if (!count) return 0;
    return my_syscall(SYS_write, fd, (long)buf, (long)count);
}
