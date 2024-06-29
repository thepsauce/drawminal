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
    Rect r;
    struct event ev;

    (void) write;

    while (1) {
        GetDialogRect(&r);
        DrawBox("Choose file", &r);

        r.x++;
        r.y++;
        r.w -= 2;
        r.h -= 2;

        for (int e = r.y + r.h; r.y < e; r.y++) {
            DrawString(stdscr, &r, 0, "FILE");
        }

        GetEvent(&ev);
        switch (ev.type) {
        case EV_KEYDOWN:
            switch (ev.key) {
            case 'q':
                return;
            }
            break;
        default:
            break;
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
        case 'w':
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

