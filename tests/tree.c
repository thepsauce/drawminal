#include "file.h"

#include <stdio.h>

int main(void)
{
	struct tree tree;

	if (InitTree(&tree, "/home") < 0) {
		return -1;
	}
	while (NextFile(&tree) == 0) {
		printf("file: %.*s\n", (int) tree.l, tree.p);
	}
	return 0;
}
