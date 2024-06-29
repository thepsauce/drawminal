#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "screen.h"
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

void FileDialog(struct canvas *cv)
{
    char *dirPath = getenv("DRAWMINAL_FILES");
    if (dirPath == NULL) {
        Panic("NOOOO");
        // TODO print error message
       return;
    }
    // int dirPathlen = strlen(dirPath);

    struct tree tree;
    struct file_list filesList;

    if (InitTree(&tree, dirPath)) {
        return;
    }
    ListFiles(&tree, &filesList);

    char **fileNames = filesList.f;
    int filesLen = filesList.nf;
    
    // Centered rectangle
    Rect r;
    GetDialogRect(&r);

    int selected = 2; // Index of the selected file (starts at 1)
    char saveNLoadMessage[512];
    while(1) {
        int c = getch();
        switch(c) {
            case 'f':
                clear();
                do {
                    DrawBox("File Explorer", &r);

                    // Input guide
                    attr_on(A_BOLD, NULL);
                    mvprintw(r.y+2, r.x+2, "use 's' to save or 'l' to load the selected file");
                    mvprintw(r.y+3, r.x+2, "use 'j' and 'k' to move down and up the files");
                    // prints path at the end
                    mvprintw(r.y+r.h-3, r.x+2, "%s", saveNLoadMessage);
                    mvprintw(r.y+r.h-2, r.x+2, "%s", dirPath);
                    attr_off(A_BOLD, NULL);

                    for(int i = 0; i < filesLen; i++) {
                        if (i+1 == selected) {
                            mvprintw(r.y+i+5, r.x+2, "(%d) %s<-", i+1, fileNames[i]); 
                        }
                        mvprintw(r.y+i+5, r.x+2, "(%d) %s", i+1, fileNames[i]); 
                    }

                    char filePath[512];
                    sprintf(filePath, "%s/%s", dirPath, fileNames[selected - 1]);
                    int c_move = getch();
                    switch(c_move) {
                        case 's':
                            do {
                                if(SaveCanvas(cv, filePath) == 0) {
                                    sprintf(saveNLoadMessage, "Canvas saved to file %s", fileNames[selected - 1]);
                                } else {
                                    sprintf(saveNLoadMessage, "Failed to save file");
                                }
                            } while(0);
                            break;
                        case 'l': 
                            do {
                                if(LoadCanvas(cv, filePath) == 0) {
                                    sprintf(saveNLoadMessage, "Canvas Loaded from file %s", fileNames[selected - 1]);
                                } else {
                                    sprintf(saveNLoadMessage, "Failed to load file");
                                }
                            } while(0);
                            break;
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
                    GetDialogRect(&r);
                    clear();
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
            FileDialog(cv);
            break;
        default:
            break;
        }
    default:
        break;
    }
    return 0;
}

