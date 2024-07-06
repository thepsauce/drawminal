#include "screen.h"
#include "ui.h"

#include <stdio.h>

int SaveCanvas(struct canvas *cv, const char *file_path)
{
    (void)cv;

    FILE *f = fopen(file_path, "wb");
    if (f == NULL) {
        Panic("Failed to save to file");
    }

    fclose(f);
    return 0;
}

int LoadCanvas(struct canvas *cv, const char *file_path) {
    (void)cv;

    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        Panic("Failed to load from file");
    }

    fclose(f);
    return 0;
}
