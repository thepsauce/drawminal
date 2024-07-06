#include "gfx.h"
#include "screen.h"
#include "ui.h"
#include "geom.h"

#include <stdio.h>
#include <stdlib.h>

int SaveCanvas(struct canvas *cv, const char *file_path) {
    FILE *f = fopen(file_path, "wb");
    if (f == NULL) {
        Panic("File points to null");
        return -1;
    }

    int max_y, max_x;
    getmaxyx(cv->data, max_y, max_x);
    int y, x;
    for (y = 0; y < max_y; ++y) {
        for (x = 0; x < max_x; ++x) {
            int ch = mvwinch(cv->data, y, x);
            fprintf(f, "%c", ch);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    return 0;
}

int LoadCanvas(struct canvas *cv, const char *file_path) {
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        Panic("Failed to load from file");
        return -1;
    }

    int max_y, max_x;
    getmaxyx(cv->data, max_y, max_x);
    int y, x;
    for (y = 0; y < max_y; ++y) {
        for (x = 0; x < max_x; ++x) {
            int ch = fgetc(f);
            if (ch == EOF) {
                Panic("Unexpected end of file");
                fclose(f);
                return -1;
            }
            mvwaddch(cv->data, y, x, ch);
        }
        int newline = fgetc(f);
        if (newline != '\n' && newline != EOF) {
            Panic("Invalid file format");
            fclose(f);
            return -1;
        }
    }

    fclose(f);
    return 0;
}

