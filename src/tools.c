#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "gfx.h"
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
    Rect r;

    char *BrushOptions[] = {"Normal", "Thick", "Erase"};

    switch (ev->type) {
    case EV_DRAW:
        r = (Rect) { Toolbar.x, Toolbar.y, Toolbar.w, Toolbar.h };
        attr_set(A_NORMAL, 0, 0);
        unsigned offset = 0;
        for (size_t i = 0; i < Brushes.n; ++i) {
            char *option = BrushOptions[i];
            if (Brushes.sel == i) {
                attr_on(A_BOLD, NULL);
                mvprintw(r.y, r.x + offset, option);
                attr_off(A_BOLD, NULL);
            } else {
                mvprintw(r.y, r.x + offset, option);
            }
            offset += strlen(option) + 1;
        }
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
        switch(ev->key) {
        case 'j': 
            if (Brushes.sel < Brushes.n - 1) Brushes.sel++;
            break;
        case 'k':
            if (Brushes.sel > 0) Brushes.sel--;
            break;
        break;
        }
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

static size_t selected = 1;
static char file_path[256];
static char action_message[512];
int fileHandle(struct tool *tool, struct canvas *cv, struct event *ev)
{
    (void) tool;

    char *dir_path = getenv("DRAWMINAL_FILES");

    struct tree *tree = malloc(sizeof(*tree));
    struct file_list *list = malloc(sizeof(*list));

    if(InitTree(tree, dir_path) != 0) {
        Panic("wtf is happening...");
    }

    ListFiles(tree, list);

    Rect r;
    GetDialogRect(&r);

    DrawBox("Files", &r);

    mvprintw(r.y + 1, r.x + 2, "use 'j' and 'k' to move, 'w' to save, 'l' to load");
    for(size_t i = 0; i < list->nf; i++) {
        if (i + 1 == selected) {
            attr_on(A_BOLD, NULL);
        }
        mvprintw(r.y + 3 + i, r.x + 2, "(%zu) %s", i + 1, list->f[i]);
        attr_off(A_BOLD, NULL);
    }

    sprintf(file_path, "%s/%s", dir_path, list->f[selected-1]);
    mvprintw(r.y + r.h - 2, r.x + 1, "%s", file_path);

    mvprintw(r.y + r.h - 3, r.x + 1, "%s", action_message);

    switch (ev->type) {
    case EV_KEYDOWN:
        switch (ev->key) {
        case 'w':
            SaveCanvas(cv, ev, file_path);
            sprintf(action_message, "Saved to %s", file_path);
            break;
        case 'l':
            LoadCanvas(cv, ev, file_path);
            sprintf(action_message, "Loaded from %s", file_path);
            break;
        case 'j':
            if (selected < list->nf) selected++;
            break;
        case 'k':
            if (selected > 1) selected--;
            break;
        case 'q':
            DestroyTree(tree);
            DestroyFileList(list);
            return 0;
            break;
        default:
            break;
        }
    default:
        break;
    }

    DestroyTree(tree);
    DestroyFileList(list);
    return 0;
}
