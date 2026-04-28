#ifndef INODE_H
#define INODE_H

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_NAME_LEN 255      // max filename length
#define MAX_CHILDREN 1024     // max children per directory

// metadata stored in each inode
typedef struct INode_Attr {
    char name[MAX_NAME_LEN + 1]; // file name
    mode_t mode;                // file type + permissions
    nlink_t nlink;              // link count
    char *data;                 // file content (regular files)
    size_t size;                // file size
    char *link_target;          // symlink target
} INode_Attr;

// in-memory inode structure (tree node)
typedef struct INode {
    struct INode *parent;
    struct INode *children[MAX_CHILDREN];
    size_t child_count;

    struct INode_Attr attr;
} INode;

extern INode root;  // root of filesystem tree

// find child inode by name under a directory
INode *find_child(INode *dir, const char *name);

// resolve full path ("/a/b/c") to inode
INode *find_by_path(const char *path);

// split path into parent inode + basename
INode *parse_parent(const char *path, char *basename);

// create and insert new inode into parent
INode *create_inode(INode *parent, const char *name, mode_t mode);

// recursively count files and data size for statfs
void collect_statfs(INode *inode, size_t *files, size_t *blocks);

#endif
