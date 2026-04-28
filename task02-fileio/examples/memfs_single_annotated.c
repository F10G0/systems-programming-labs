#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/statvfs.h>

// ----------------------
// Logging (persistence)
// ----------------------

#define LOG_FILENAME "log"

// Log file path (outside mountpoint)
static char log_path[1024];

// Prevent logging during replay phase
static bool is_replaying;

// Build log file path based on mountpoint
// e.g. /tmp/mnt → /tmp/log
static void init_log_path(char *mount_path) {
    snprintf(log_path, sizeof(log_path), "%s", mount_path);

    // strip last component (mount dir itself)
    char *slash = strrchr(log_path, '/');
    if (slash) *slash = '\0';

    // append log filename
    strncat(log_path, "/" LOG_FILENAME,
            sizeof(log_path) - strlen(log_path) - 1);
}

// Append one filesystem-changing operation to log
static void append_log(char op, const char *path, mode_t mode,
                       const char *buf, size_t size,
                       off_t offset, const char *target) {

    // Do not log while replaying
    if (is_replaying) return;

    FILE *fp = fopen(log_path, "a");
    if (!fp) return;

    switch(op) {
        case 'M': // mkdir
            fprintf(fp, "M %s %o\n", path, mode);
            break;

        case 'C': // create file
            fprintf(fp, "C %s %o\n", path, mode);
            break;

        case 'W': // write
            fprintf(fp, "W %s %ld %zu ", path, (long)offset, size);

            // store binary data as hex (text-safe)
            for (size_t i = 0; i < size; i++) {
                fprintf(fp, "%02x", (unsigned char)buf[i]);
            }
            fprintf(fp, "\n");
            break;

        case 'S': // symlink
            fprintf(fp, "S %s %s\n", path, target);
            break;

        default:
            break;
    }

    fclose(fp);
}

// ----------------------
// In-memory filesystem
// ----------------------

#define MAX_NAME_LEN 255
#define MAX_CHILDREN 1024

// Metadata per inode
typedef struct INode_Attr {
    char name[MAX_NAME_LEN + 1]; // filename
    mode_t mode;                // type + permissions
    nlink_t nlink;              // link count
    char *data;                 // file content (regular files)
    size_t size;                // file size
    char *link_target;          // symlink target
} INode_Attr;

// Tree node representing file/directory
typedef struct INode {
    struct INode *parent;
    struct INode *children[MAX_CHILDREN];
    size_t child_count;

    struct INode_Attr attr;
} INode;

// Root directory
static INode root = {
    .parent = NULL,
    .child_count = 0,
    .attr = {
        .name = "",
        .mode = S_IFDIR | 0755,
        .nlink = 2, // "." and ".."
        .data = NULL,
        .size = 0,
        .link_target = NULL
    }
};

// Find child inode by name
static INode *find_child(INode *dir, const char *name) {
    if (!dir) return NULL;
    if (!S_ISDIR(dir->attr.mode)) return NULL;

    for (size_t i = 0; i < dir->child_count; i++) {
        INode *child = dir->children[i];
        if (child && strcmp(child->attr.name, name) == 0)
            return child;
    }
    return NULL;
}

// Resolve full path by walking from root
static INode *find_by_path(const char *path) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s", path);

    INode *cur = &root;

    // strtok modifies string → use local copy
    char *token = strtok(buf, "/");
    while(token) {
        cur = find_child(cur, token);
        if (!cur) return NULL;
        token = strtok(NULL, "/");
    }
    return cur;
}

// Extract parent directory and basename
static INode *parse_parent(const char *path, char *basename) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s", path);

    char *p = strrchr(buf, '/');
    if (!p || !p[1]) return NULL;

    // basename = last component
    snprintf(basename, MAX_NAME_LEN + 1, "%s", p + 1);

    // "/foo" → parent is root
    if (p == buf)
        return &root;

    *p = '\0'; // terminate parent path
    return find_by_path(buf);
}

// Create inode and insert into parent
static INode *create_inode(INode *parent, const char *name, mode_t mode) {
    if (!parent || !S_ISDIR(parent->attr.mode) ||
        parent->child_count >= MAX_CHILDREN)
        return NULL;

    INode *inode = calloc(1, sizeof(INode));
    if (!inode) return NULL;

    *inode = (INode) {
        .parent = parent,
        .child_count = 0,
        .attr = {
            .mode = mode,
            .nlink = S_ISDIR(mode) ? 2 : 1,
            .data = NULL,
            .size = 0,
            .link_target = NULL
        }
    };

    snprintf(inode->attr.name, MAX_NAME_LEN + 1, "%s", name);

    parent->children[parent->child_count++] = inode;
    return inode;
}

// Traverse tree to compute stats
static void collect_statfs(INode *inode, size_t *files, size_t *blocks) {
    if (!inode) return;

    if (S_ISREG(inode->attr.mode)) {
        (*files)++;
        *blocks += inode->attr.size;
    }

    if (S_ISDIR(inode->attr.mode)) {
        for (size_t i = 0; i < inode->child_count; i++) {
            collect_statfs(inode->children[i], files, blocks);
        }
    }
}

// ----------------------
// FUSE operations
// ----------------------

// Return file metadata
static int my_fuse_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    INode *inode = find_by_path(path);
    if (!inode)
        return -ENOENT;

    stbuf->st_mode = inode->attr.mode;
    stbuf->st_nlink = inode->attr.nlink;
    stbuf->st_size = inode->attr.size;
    return 0;
}

// Read file data
static int my_fuse_read(const char *path, char *buf, size_t size,
                        off_t offset, struct fuse_file_info *fi) {

    INode *file = find_by_path(path);
    if (!file)
        return -ENOENT;
    if (!S_ISREG(file->attr.mode))
        return -EISDIR;
    if (fi && (fi->flags & O_ACCMODE) == O_WRONLY)
        return -EACCES;

    if (offset < 0)
        return -EINVAL;

    if (offset >= file->attr.size)
        return 0;

    // clamp read to EOF
    if (offset + size > file->attr.size)
        size = file->attr.size - offset;

    memcpy(buf, file->attr.data + offset, size);
    return size;
}

// Write file data
static int my_fuse_write(const char *path, const char *buf, size_t size,
                         off_t offset, struct fuse_file_info *fi) {

    INode *file = find_by_path(path);
    if (!file)
        return -ENOENT;
    if (!S_ISREG(file->attr.mode))
        return -EISDIR;
    if (fi && (fi->flags & O_ACCMODE) == O_RDONLY)
        return -EACCES;

    if (offset < 0)
        return -EINVAL;

    size_t new_size = offset + size;

    // grow buffer if needed
    if (new_size > file->attr.size) {
        char *new_data = realloc(file->attr.data, new_size);
        if (!new_data)
            return -ENOMEM;
        file->attr.data = new_data;
        file->attr.size = new_size;
    }

    memcpy(file->attr.data + offset, buf, size);

    append_log('W', path, 0, buf, size, offset, NULL);
    return size;
}

// List directory contents
static int my_fuse_readdir(const char *path, void *buf,
                           fuse_fill_dir_t filler,
                           off_t offset, struct fuse_file_info *fi) {

    INode *dir = find_by_path(path);
    if (!dir)
        return -ENOENT;
    if (!S_ISDIR(dir->attr.mode))
        return -ENOTDIR;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    for (size_t i = 0; i < dir->child_count; i++) {
        INode *child = dir->children[i];
        filler(buf, child->attr.name, NULL, 0);
    }
    return 0;
}

// Create directory
static int my_fuse_mkdir(const char *path, mode_t mode) {
    char basename[MAX_NAME_LEN + 1];
    INode *parent = parse_parent(path, basename);

    if (!parent)
        return -ENOENT;
    if (!S_ISDIR(parent->attr.mode))
        return -ENOTDIR;
    if (find_child(parent, basename))
        return -EEXIST;

    if (!create_inode(parent, basename, S_IFDIR | mode))
        return -ENOMEM;

    append_log('M', path, mode, NULL, 0, 0, NULL);
    return 0;
}

// Create file
static int my_fuse_mknod(const char *path, mode_t mode, dev_t rdev) {
    char basename[MAX_NAME_LEN + 1];
    INode *parent = parse_parent(path, basename);

    if (!parent)
        return -ENOENT;
    if (!S_ISDIR(parent->attr.mode))
        return -ENOTDIR;
    if (find_child(parent, basename))
        return -EEXIST;

    if (!create_inode(parent, basename, S_IFREG | mode))
        return -ENOMEM;

    append_log('C', path, mode, NULL, 0, 0, NULL);
    return 0;
}

// Open file (validation only)
static int my_fuse_open(const char *path, struct fuse_file_info *fi) {
    INode *file = find_by_path(path);
    if (!file)
        return -ENOENT;
    if (!S_ISREG(file->attr.mode))
        return -EISDIR;
    return 0;
}

// create = mknod
static int my_fuse_create(const char *path, mode_t mode,
                          struct fuse_file_info *fi) {
    return my_fuse_mknod(path, mode, 0);
}

// Read symlink target
static int my_fuse_readlink(const char *path, char *buf, size_t size) {
    INode *inode = find_by_path(path);
    if (!inode)
        return -ENOENT;

    if (!S_ISLNK(inode->attr.mode) || !inode->attr.link_target)
        return -EINVAL;

    snprintf(buf, size, "%s", inode->attr.link_target);
    return 0;
}

// Create symlink
static int my_fuse_symlink(const char *target, const char *linkpath) {
    char name[MAX_NAME_LEN + 1];
    INode *parent = parse_parent(linkpath, name);

    if (!parent)
        return -ENOENT;
    if (!S_ISDIR(parent->attr.mode))
        return -ENOTDIR;
    if (find_child(parent, name))
        return -EEXIST;

    INode *inode = create_inode(parent, name, S_IFLNK | 0777);
    if (!inode)
        return -ENOMEM;

    inode->attr.link_target = strdup(target);
    if (!inode->attr.link_target)
        return -ENOMEM;

    append_log('S', linkpath, 0, NULL, 0, 0, target);
    return 0;
}

// Return filesystem statistics
static int my_fuse_statfs(const char *path, struct statvfs *stbuf) {
    memset(stbuf, 0, sizeof(struct statvfs));

    size_t files = 0;
    size_t blocks = 0;
    collect_statfs(&root, &files, &blocks);

    stbuf->f_files = files;
    stbuf->f_namemax = MAX_NAME_LEN;
    stbuf->f_blocks = blocks;
    return 0;
}

// FUSE operations table
static struct fuse_operations my_fuse_ops = {
    .getattr = my_fuse_getattr,
    .read = my_fuse_read,
    .write = my_fuse_write,
    .readdir = my_fuse_readdir,
    .mkdir = my_fuse_mkdir,
    .mknod = my_fuse_mknod,
    .open = my_fuse_open,
    .create = my_fuse_create,
    .readlink = my_fuse_readlink,
    .symlink = my_fuse_symlink,
    .statfs = my_fuse_statfs,
};

// Replay log to rebuild filesystem
static void load_log(void) {
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
                if (sscanf(line, "M %s %o", path, &mode) == 2)
                    my_fuse_mkdir(path, mode);
                break;

            case 'C':
                if (sscanf(line, "C %s %o", path, &mode) == 2)
                    my_fuse_mknod(path, mode, 0);
                break;

            case 'W':
                if (sscanf(line, "W %s %ld %zu %s",
                           path, &offset, &size, hex_buf) == 4) {

                    // decode hex back to bytes
                    for (size_t i = 0; i < size; i++) {
                        unsigned int x;
                        sscanf(hex_buf + 2*i, "%2x", &x);
                        buf[i] = (char)x;
                    }

                    my_fuse_write(path, buf, size, (off_t)offset, NULL);
                }
                break;

            case 'S':
                if (sscanf(line, "S %s %s", path, target) == 2)
                    my_fuse_symlink(target, path);
                break;
        }
    }

    fclose(fp);
}

// Entry point
int main(int argc, char **argv) {
    init_log_path(argv[argc - 1]);

    // Restore state before mounting
    is_replaying = true;
    load_log();
    is_replaying = false;

    return fuse_main(argc, argv, &my_fuse_ops, NULL);
}
