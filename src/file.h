#ifndef FILE_H
#define FILE_H

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct tree {
    struct tree_stack {
        /* file descriptor of below directory */
        int fd;
        DIR *dir;
        size_t ps;
    } *stack;
    /* size and capacity of the stack */
    unsigned n, c;
    /* current path */
    char *p;
    /* length of the current path */
    size_t l;
    /* index of the directory */
    size_t ds;
    /* current directory */
    int fd;
    DIR *dir;
    struct dirent *ent;
};

struct file_list {
    /* all names are relative to this file descriptor */
    int root;
    /* file names */
    char **f;
    /* directory names */
    char **d;
    /* number of files */
    uint32_t nf;
    /* number of directories */
    uint32_t nd;
};

int InitTree(struct tree *tree, const char *root);
int ListFiles(struct tree *tree, struct file_list *list);
int CatTree(struct tree *tree, const char *name, bool dir);
int UpTree(struct tree *tree);
int ClimbTree(struct tree *tree, const char *name);
int NextFile(struct tree *tree);

#endif
