#ifndef MEMFS_H
#define MEMFS_H

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stddef.h>

// FUSE callback declarations
int my_fuse_getattr(const char *path, struct stat *stbuf);
int my_fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int my_fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int my_fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int my_fuse_mkdir(const char *path, mode_t mode);
int my_fuse_mknod(const char *path, mode_t mode, dev_t rdev);
int my_fuse_open(const char *path, struct fuse_file_info *fi);
int my_fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int my_fuse_readlink(const char *path, char *buf, size_t size);
int my_fuse_symlink(const char *target, const char *linkpath);
int my_fuse_statfs(const char *path, struct statvfs *stbuf);

#endif
