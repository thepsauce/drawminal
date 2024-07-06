#include "screen.h"
#include "ui.h"

#include <ncurses.h>
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

    int h, w;
    getmaxyx(cv->data, h, w);
    fprintf(f, "%d %d ", h, w);

    chtype c = mvwinch(cv->data, 0, 0), current_c;
    attr_t a = c & A_ATTRIBUTES, current_a;
    unsigned count = 0;
    unsigned total_count = 0;
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            current_c = mvwinch(cv->data, y, x); 
            current_a = current_c & A_ATTRIBUTES;
            if (c == current_c && a == current_a) {
                count++;
            } else {
                fprintf(f, "(%c|%d|%d)", c, a, count);
                c = current_c;
                a = current_a;
                total_count += count;
                count = 1;
            }
        }
    }
    fprintf(f, "(%c|%d|%d)", ' ', a, (w*h) - total_count);

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

    unsigned y_max, x_max;
    fscanf(f, "%d %d ", &y_max, &x_max);

    unsigned x = 0, y = 0;
    while(y < y_max) {
        char c;
        attr_t a;
        unsigned count;
        fscanf(f, "(%c|%d|%d)", &c, &a, &count);
        for(size_t i = 0; i < count; i++) {
            mvwaddch(cv->data, y, x, c | a);
            if (x == x_max - 1) {
                x = 0;
                y++;
            } else {
                x++;
            }
        }
    }

    cvHandle(cv, ev);
    fclose(f);
    return 0;
}
