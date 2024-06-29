#include "screen.h"
#include "macros.h"
#include "file.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

static inline int openat_dir(int at, const char *name, int *pfd, DIR **pdir)
{
    int fd;
    DIR *dir;

    fd = openat(at, name, O_DIRECTORY | O_RDONLY);
    if (fd == -1) {
        Dialog("Error", "could not open %s: %s",
                strerror(errno), name, "[O]k", NULL);
        return -1;
    }
    dir = fdopendir(fd);
    if (dir == NULL) {
        close(fd);
        return -1;
    }
    *pfd = fd;
    *pdir = dir;
    return 0;
}

int InitTree(struct tree *tree, const char *root)
{
    memset(tree, 0, sizeof(*tree));
    tree->p = Malloc(256);
    if (tree->p == NULL) {
        return -1;
    }
    return openat_dir(0, root, &tree->fd, &tree->dir);
}

static inline struct dirent *read_dir(DIR *dir)
{
    struct dirent *ent;

    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') {
            if (ent->d_name[1] == '\0') {
                continue;
            }
            if (ent->d_name[1] == '.' && ent->d_name[2] == '\0') {
                continue;
            }
        }
        break;
    }
    return ent;
}

int ListFiles(struct tree *tree, struct file_list *list)
{
    char **files = NULL;
    char **dirs = NULL;
    uint32_t nf = 0, nd = 0;
    char **p;
    struct dirent *ent;
    long pos;

    pos = telldir(tree->dir);
    rewinddir(tree->dir);

    while ((ent = read_dir(tree->dir)) != NULL) {
        if (ent->d_type == DT_REG) {
            p = Realloc(files, sizeof(*files) * (nf + 1));
            if (p == NULL) {
                goto err;
            }
            files = p;
            files[nf++] = Strdup(ent->d_name);
        } else if (ent->d_type == DT_DIR) {
            p = Realloc(dirs, sizeof(*dirs) * (nd + 1));
            if (p == NULL) {
                goto err;
            }
            dirs = p;
            dirs[nd++] = Strdup(ent->d_name);
        }
    }

    seekdir(tree->dir, pos);

    list->root = tree->fd;
    list->f = files;
    list->d = dirs;
    list->nf = nf;
    list->nd = nd;
    return 0;

err:
    for (uint32_t i = 0; i < nf; i++) {
        Free(files[i]);
    }
    for (uint32_t i = 0; i < nd; i++) {
        Free(dirs[i]);
    }
    Free(files);
    Free(dirs);
    return -1;
}

int CatTree(struct tree *tree, const char *name, bool dir)
{
    size_t lenName;
    char *newPath;

    lenName = strlen(name);
    newPath = Realloc(tree->p, tree->ds + lenName + 1);
    if (newPath == NULL) {
        return -1;
    }
    tree->p = newPath;
    memcpy(&tree->p[tree->ds], name, lenName);
    tree->l = tree->ds + lenName;
    if (dir) {
        tree->p[tree->l++] = '/';
        tree->ds = tree->l;
    }
    return 0;
}

int UpTree(struct tree *tree)
{
    if (tree->n == 0) {
        return 1;
    }
    closedir(tree->dir);
    tree->n--;
    tree->fd = tree->stack[tree->n].fd;
    tree->dir = tree->stack[tree->n].dir;
    tree->l = tree->stack[tree->n].ps;
    tree->ds = tree->l;
    return 0;
}

int ClimbTree(struct tree *tree, const char *name)
{
    int fd;
    DIR *dir;
    size_t ds;

    if (openat_dir(tree->fd, name, &fd, &dir) < 0) {
        return -1;
    }
    ds = tree->ds;
    if (CatTree(tree, name, true) < 0) {
        closedir(dir);
        return -1;
    }
    if (tree->n + 1 > tree->c) {
        tree->c *= 2;
        tree->c++;
        struct tree_stack *const p =
            Realloc(tree->stack, sizeof(*tree->stack) * tree->c);
        if (p == NULL) {
            closedir(dir);
            return -1;
        }
        tree->stack = p;
    }
    tree->stack[tree->n].fd = tree->fd;
    tree->stack[tree->n].dir = tree->dir;
    tree->stack[tree->n].ps = ds;
    tree->n++;
    tree->fd = fd;
    tree->dir = dir;
    return 0;
}

int NextFile(struct tree *tree)
{
    struct dirent *ent;

    ent = read_dir(tree->dir);

next:
    while (ent == NULL) {
        if (UpTree(tree) == 1) {
            return 1;
        }
        ent = read_dir(tree->dir);
    }

    while (ent->d_type == DT_DIR) {
        if (ClimbTree(tree, ent->d_name) < 0) {
            ent = read_dir(tree->dir);
            goto next;
        }
        ent = read_dir(tree->dir);
        if (ent == NULL) {
            goto next;
        }
    }
    if (CatTree(tree, ent->d_name, false) < 0) {
        goto next;
    }
    tree->ent = ent;
    return 0;
}
