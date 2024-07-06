#include "screen.h"
#include "ui.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int cvHandle(struct canvas *cv, struct event *ev);

int SaveCanvas(struct canvas *cv, struct event *ev, const char *file_path)
{
    (void)ev;

    FILE *f = fopen(file_path, "wb");
    if (f == NULL) {
        Panic("Failed to save to file");
    }
    if (truncate(file_path, 0) != 0) {
        perror("Error truncating file");
        return 1;
    }

    fputc(cv->h, f);
    fputc(cv->w, f);

    for(int y = 0; y < cv->h; y++) {
        fputc('\n', f);
        for(int x = 0; x < cv->w; x++) {
            chtype c = mvwinch(cv->data, y, x); 
            attr_t a = c & A_ATTRIBUTES;
            fprintf(f, "%c %d", c, a);
        }
    }

    fclose(f);
    return 0;
}

int LoadCanvas(struct canvas *cv, struct event *ev, const char *file_path) {
    (void)cv;

    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        Panic("Failed to load from file");
    }

    clear();

    int y_max = fgetc(f);
    int x_max = fgetc(f);

    for(int y = 0; y < y_max; y++) {
        for(int x = 0; x < x_max; x++) {
            char c;
            attr_t a;
            fscanf(f, "%c %d", &c, &a);
            mvwaddch(cv->data, y, x, c | a);
        }
        // fgetc(f);
    }

    cvHandle(cv, ev);
    fclose(f);
    return 0;
}
