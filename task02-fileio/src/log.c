#include "log.h"
#include "memfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_FILENAME "log"

char log_path[1024];
bool is_replaying;

// Store log next to the mountpoint so remounts share the same state
void init_log_path(const char *mount_path) {
    snprintf(log_path, sizeof(log_path), "%s", mount_path);

    // Use parent directory of mountpoint, not the mounted directory itself
    char *slash = strrchr(log_path, '/');
    if (slash) *slash = '\0';

    strncat(log_path, "/" LOG_FILENAME, sizeof(log_path) - strlen(log_path) - 1);
}

// Append one mutating operation to the persistence log
void append_log(char op, const char *path, mode_t mode, const char *buf, size_t size, off_t offset, const char *target) {
    if (is_replaying) return; // avoid logging operations during recovery

    FILE *fp = fopen(log_path, "a");
    if (!fp) return;

    switch(op) {
        case 'M':
            fprintf(fp, "M %s %o\n", path, mode);
            break;
        case 'C':
            fprintf(fp, "C %s %o\n", path, mode);
            break;
        case 'W':
            fprintf(fp, "W %s %ld %zu ", path, (long)offset, size);

            // Encode raw bytes as hex to keep log text-safe
            for (size_t i = 0; i < size; i++) {
                fprintf(fp, "%02x", (unsigned char)buf[i]);
            }
            fprintf(fp, "\n");
            break;
        case 'S':
            fprintf(fp, "S %s %s\n", path, target);
            break;
        default:
            break;
    }

    fclose(fp);
}

// Replay all logged operations to reconstruct the in-memory tree
void load_log(void) {
    FILE *fp = fopen(log_path, "r");
    if (!fp) return;

    char line[4096];
    mode_t mode;
    char path[1024];
    long offset;
    size_t size;
    char hex_buf[4096];
    char buf[2048];
    char target[1024];

    while (fgets(line, sizeof(line), fp)) {
        char op;
        if (sscanf(line, "%c", &op) != 1) continue;

        switch (op) {
            case 'M':
                if (sscanf(line, "M %s %o", path, &mode) == 2) {
                    my_fuse_mkdir(path, mode);
                }
                break;
            case 'C':
                if (sscanf(line, "C %s %o", path, &mode) == 2) {
                    my_fuse_mknod(path, mode, 0);
                }
                break;
            case 'W':
                if (sscanf(line, "W %s %ld %zu %s", path, &offset, &size, hex_buf) == 4) {
                    // Decode hex back into original file bytes
                    for (size_t i = 0; i < size; i++) {
                        unsigned int x;
                        sscanf(hex_buf + 2*i, "%2x", &x);
                        buf[i] = (char)x;
                    }

                    my_fuse_write(path, buf, size, (off_t)offset, NULL);
                }
                break;
            case 'S':
                if (sscanf(line, "S %s %s", path, target) == 2) {
                    my_fuse_symlink(target, path);
                }
                break;
        }
    }

    fclose(fp);
}
