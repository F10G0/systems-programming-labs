#include "inode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Root directory inode (single-root filesystem)
INode root = {
    .parent = NULL,
    .child_count = 0,

    .attr = {
        .name = "",
        .mode = S_IFDIR | 0755,
        .nlink = 2,
        .data = NULL,
        .size = 0,
        .link_target = NULL
    }
};

// Find a child inode by name inside a directory
INode *find_child(INode *dir, const char *name) {
    if (!dir) return NULL;
    if (!S_ISDIR(dir->attr.mode)) return NULL;

    for (size_t i = 0; i < dir->child_count; i++) {
        INode *child = dir->children[i];
        if (child && strcmp(child->attr.name, name) == 0)
            return child;
    }
    return NULL;
}

// Resolve path by walking the inode tree from root
INode *find_by_path(const char *path) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s", path);

    INode *cur = &root;

    // strtok modifies buffer, so we work on a copy
    char *token = strtok(buf, "/");
    while(token) {
        cur = find_child(cur, token);
        if (!cur) return NULL; // path component not found
        token = strtok(NULL, "/");
    }
    return cur;
}

// Split path into parent directory and final component
INode *parse_parent(const char *path, char *basename) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s", path);

    char *p = strrchr(buf, '/');
    if (!p || !p[1]) return NULL;

    // basename = last component after '/'
    snprintf(basename, MAX_NAME_LEN + 1, "%s", p + 1);

    // special case: parent is root ("/foo")
    if (p == buf)
        return &root;

    *p = '\0'; // terminate to get parent path
    return find_by_path(buf);
}

// Allocate a new inode and attach it to parent
INode *create_inode(INode *parent, const char *name, mode_t mode) {
    if (!parent || !S_ISDIR(parent->attr.mode) || parent->child_count >= MAX_CHILDREN)
        return NULL;

    INode *inode = calloc(1, sizeof(INode)); // zero-initialize
    if (!inode) return NULL;

    *inode = (INode) {
        .parent = parent,
        .child_count = 0,
        .attr = {
            .mode = mode,
            .nlink = S_ISDIR(mode) ? 2 : 1, // directories start with 2 links
            .data = NULL,
            .size = 0,
            .link_target = NULL
        }
    };

    snprintf(inode->attr.name, MAX_NAME_LEN + 1, "%s", name);

    // append to parent's children list
    parent->children[parent->child_count++] = inode;

    return inode;
}

// Count regular files and total file-data bytes
void collect_statfs(INode *inode, size_t *files, size_t *blocks) {
    if (!inode)
        return;

    if (S_ISREG(inode->attr.mode)) {
        (*files)++;
        *blocks += inode->attr.size;
    }

    // recursively traverse directory tree
    if (S_ISDIR(inode->attr.mode)) {
        for (size_t i = 0; i < inode->child_count; i++) {
            collect_statfs(inode->children[i], files, blocks);
        }
    }
}
