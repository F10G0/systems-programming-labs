#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

// Wrapper implementation of read/write using libc syscall()
// This replaces the default glibc wrapper but keeps identical behavior.
ssize_t read(int fd, void *buf, size_t count) {
    // Optimization: avoid syscall when count == 0
    if (!count) return 0;
    // Forward directly to kernel via syscall()
    return syscall(SYS_read, fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    if (!count) return 0;
    return syscall(SYS_write, fd, buf, count);
}
