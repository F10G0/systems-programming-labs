#include "memfs.h"
#include "inode.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

// Register implemented FUSE operations
struct fuse_operations my_fuse_ops = {
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

// Fill stat metadata for a path
int my_fuse_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    INode *inode = find_by_path(path);
    if (!inode)
        return -ENOENT;

    stbuf->st_mode = inode->attr.mode;
    stbuf->st_nlink = inode->attr.nlink;
    stbuf->st_size = inode->attr.size;
    return 0;
}

// Read data from regular file buffer
int my_fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
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

    // Clamp read size at EOF
    if (offset + size > file->attr.size)
        size = file->attr.size - offset;
    memcpy(buf, file->attr.data + offset, size);
    return size;
}

// Write data, grow file if necessary, then persist operation
int my_fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
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

// List entries in a directory
int my_fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
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

// Create directory inode
int my_fuse_mkdir(const char *path, mode_t mode) {
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

// Create regular file inode
int my_fuse_mknod(const char *path, mode_t mode, dev_t rdev) {
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

// Validate opening a regular file
int my_fuse_open(const char *path, struct fuse_file_info *fi) {
    INode *file = find_by_path(path);
    if (!file)
        return -ENOENT;
    if (!S_ISREG(file->attr.mode))
        return -EISDIR;
    return 0;
}

// create = mknod + open for this simple filesystem
int my_fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    return my_fuse_mknod(path, mode, 0);
}

// Return symlink target
int my_fuse_readlink(const char *path, char *buf, size_t size) {
    INode *inode = find_by_path(path);
    if (!inode)
        return -ENOENT;

    if (!S_ISLNK(inode->attr.mode) || !inode->attr.link_target)
        return -EINVAL;
    snprintf(buf, size, "%s", inode->attr.link_target);
    return 0;
}

// Create symlink inode and store target path
int my_fuse_symlink(const char *target, const char *linkpath) {
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

// Report simple filesystem statistics
int my_fuse_statfs(const char *path, struct statvfs *stbuf) {
    memset(stbuf, 0, sizeof(struct statvfs));

    size_t files = 0;
    size_t blocks = 0;
    collect_statfs(&root, &files, &blocks);

    stbuf->f_files = files;
    stbuf->f_namemax = MAX_NAME_LEN;
    stbuf->f_blocks = blocks;
    return 0;
}

int main(int argc, char **argv) {
    init_log_path(argv[argc - 1]);

    // Rebuild state before mounting and disable logging during replay
    is_replaying = true;
    load_log();
    is_replaying = false;

    return fuse_main(argc, argv, &my_fuse_ops, NULL);
}
