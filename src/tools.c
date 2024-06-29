#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "ui.h"

/*
 * implementation of various little tools
 * brush, pipet, file
 */

void brHandle(struct tool *tool, struct canvas *cv, struct event *ev)
{
    struct hist_event *hev;
    struct brush *br;

    switch (ev->type) {
    case EV_DRAW:
        br = Brushes.p[Brushes.sel];
        if (tool->st.n == 0) {
            DrawPatLine(br->pat, stdscr, Mouse.x, Mouse.y, Mouse.x, Mouse.y, br->o);
        } else {
            tool->st.br = br;
            DrawStroke(&tool->st, stdscr, cv->x, cv->y);
        }
        break;
    case EV_MOUSEMOVE:
        if (tool->st.n == 0) {
            break;
        }
        if (Mouse.bt[BT_LEFT]) {
    case EV_LBUTTONDOWN:
            AddPoint(&tool->st, (Point) { Mouse.x - cv->x, Mouse.y - cv->y });
        }
        break;
    case EV_RBUTTONDOWN:
        tool->st.n = 0;
        break;
    case EV_LBUTTONUP:
        if (tool->st.n == 0) {
            break;
        }
        tool->st.rect.w++;
        tool->st.rect.h++;
        tool->st.cache = Newpad(tool->st.rect.h, tool->st.rect.w);
        if (tool->st.cache != NULL) {
            copywin(cv->data, tool->st.cache, tool->st.rect.y, tool->st.rect.x,
                0, 0, tool->st.rect.h - 1, tool->st.rect.w - 1, false);
        }
        DrawStroke(&tool->st, cv->data, 0, 0);

        hev = CreateEvent(&cv->hist, HEV_STROKE, NULL);
        if (hev != NULL) {
            hev->st = tool->st;
            hev->st.p = Malloc(sizeof(*hev->st.p) * tool->st.n);
            if (hev->st.p == NULL) {
                DropEvent(&cv->hist);
            } else {
                hev->st.n = tool->st.n;
                memcpy(hev->st.p, tool->st.p,
                    sizeof(*hev->st.p) * hev->st.n);
            }
        }
        tool->st.n = 0;
        break;
    default:
        break;
    }
}

int piHandle(struct tool *tool, struct canvas *cv, struct event *ev)
{
    chtype ct;

    (void) tool;

    switch (ev->type) {
    case EV_DRAW:
        if (RectContains(&(Rect) { cv->x, cv->y, cv->w, cv->h },
                &(Point) { Mouse.x, Mouse.y })) {
            mvaddch(Mouse.y, Mouse.x, '*');
        }
        break;
    case EV_LBUTTONDOWN:
        ct = mvwinch(cv->data, Mouse.y - cv->y, Mouse.x - cv->x);
        SetBrushColor(Brushes.p[Brushes.sel], PAIR_NUMBER(ct));
        break;
    default:
        break;
    }
    return 0;
}

void FileDialog(bool write)
{
    (void) write;

    char *dirPath = getenv("DRAWMINAL_FILES");
    if (dirPath == NULL) {
        // TODO print error message
       return;
    }
    // int dirPathlen = strlen(dirPath);

    struct tree tree;

    if (InitTree(&tree, dirPath)) {
        return;
    }

    char **fileNames = malloc(sizeof(char **));
    int filesLen = 0;
    while (NextFile(&tree) == 0) {
        char buffer[128];
        sprintf(buffer, "%.*s\n", (int) tree.l, tree.p);

        filesLen++;
        fileNames = realloc(fileNames, sizeof(char *) * filesLen);
        fileNames[filesLen - 1] = buffer;
    }
    
    // Centered rectangle
    Rect r = {COLS/2 - COLS/4, LINES/2 - LINES/4, COLS/2, LINES/2};

    int selected = 2; // Index of the selected file (starts at 1)
    while(1) {
        int c = getch();
        switch(c) {
            case 'w':
                clear();
                do {
                    DrawBox("File Explorer", &r);

                    // Input guide
                    attr_on(A_BOLD, NULL);
                    mvprintw(r.y+2, r.x+2, "use 's' to save to the selected file");
                    mvprintw(r.y+3, r.x+2, "use 'j' and 'k' to move down and up the files");
                    // prints path at the end
                    mvprintw(r.y+r.h-2, r.x+2, "%s", dirPath);
                    attr_off(A_BOLD, NULL);

                    for(int i = 0; i < filesLen; i++) {
                        if (i+1 == selected) {
                            mvprintw(r.y+i+5, r.x+2, "(%d) %s %d <-", i+1, fileNames[i], i+1); 
                        }
                        mvprintw(r.y+i+5, r.x+2, "(%d) %s %d", i+1, fileNames[i], i+1); 
                    }

                    int c_move = getch();
                    switch(c_move) {
                        case 'j':
                            if (selected < filesLen) selected++;
                            break;
                        case 'k':
                            if (selected > 1) selected--;
                            break;
                        case 'q':
                            free(fileNames);
                            return;
                            break;
                        default:
                            break;
                    }
                } while(1);
                break;
            case 'q': 
                return;
                break;
            default:
                continue;
        }
    }
}

int fileHandle(struct tool *tool, struct canvas *cv, struct event *ev)
{
    (void) tool;
    (void) cv;

    switch (ev->type) {
    case EV_KEYDOWN:
        switch (ev->key) {
        case 'f':
            FileDialog(true);
            break;
        case 'r':
            FileDialog(false);
            break;
        default:
            break;
        }
    default:
        break;
    }
    return 0;
}

