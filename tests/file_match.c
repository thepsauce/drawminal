#include "file.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    struct tree tree;
    size_t len;
    size_t c = 0;

    if (argc != 2) {
        printf("usage:\n  %s <name>\n", argv[0]);
        return 1;
    }

    if (InitTree(&tree, "/") < 0) {
        return -1;
    }
    len = strlen(argv[1]);
    while (NextFile(&tree) == 0) {
        if (strncmp(&tree.p[tree.ds], argv[1], len) == 0) {
            printf("%.*s\n", (int) tree.l, tree.p);
            c++;
        }
    }
    printf("found %zu files\n", c);
    return 0;
}
