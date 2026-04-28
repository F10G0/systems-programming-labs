#ifndef LOG_H
#define LOG_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdbool.h>

// Initialize persistent log path based on mount location
void init_log_path(const char *mount_path);

// Append one filesystem-changing operation to the log
void append_log(char op, const char *path, mode_t mode, const char *buf, size_t size, off_t offset, const char *target);

// Replay log entries to rebuild in-memory state
void load_log(void);

// Prevent replayed operations from being logged again
extern bool is_replaying;

// Persistent log file path
extern char log_path[1024];

#endif
