#include "macros.h"
#include "screen.h"
#include "ui.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

static bool UIRunning;

int brHandle(struct tool *tool, struct canvas *cv, struct event *ev);
int piHandle(struct tool *tool, struct canvas *cv, struct event *ev);
int seHandle(struct tool *tool, struct canvas *cv, struct event *ev);
int fileHandle(struct tool *tool, struct canvas *cv, struct event *ev);

int cvHandle(struct canvas *cv, struct event *ev);

void cpHandle(struct event *ev);
void tbHandle(struct event *ev);

void RenderUI(void)
{
    struct event ev;

    if (!UIRunning) {
        return;
    }

    erase();

    ColorPicker.x = 0;
    ColorPicker.y = 0;
    ColorPicker.w = COLS;
    ColorPicker.h = 1;

    if (COLS <= 3 || LINES <= 3) {
        Canvases.p[Canvases.sel].x = 0;
        Canvases.p[Canvases.sel].y = 1;
        Canvases.p[Canvases.sel].w = COLS;
        Canvases.p[Canvases.sel].h = LINES;

        Brushes.w = 0;
    } else {
        Canvases.p[Canvases.sel].x = 0;
        Canvases.p[Canvases.sel].y = 1;
        Canvases.p[Canvases.sel].w = Brushes.v ? COLS * 2 / 3 : COLS;
        Canvases.p[Canvases.sel].h = LINES - 3;

        Brushes.x = Canvases.p[Canvases.sel].w;
        Brushes.y = 1;
        Brushes.w = COLS - Canvases.p[Canvases.sel].w;
        Brushes.h = Canvases.p[Canvases.sel].h;

        Toolbar.x = 4;
        Toolbar.y = LINES - 2;
        Toolbar.w = COLS;
        Toolbar.h = 2;
    }

    ev.key = 0;
    ev.type = EV_DRAW;
    cvHandle(&Canvases.p[Canvases.sel], &ev);
    Toolbar.t[Toolbar.sel].handle(&Toolbar.t[Toolbar.sel],
            &Canvases.p[Canvases.sel], &ev);
    tbHandle(&ev);
    cpHandle(&ev);
}

int RunUI(void)
{
    /* stock tools / default tools*/
    struct tool stools[] = {
        { "_file", fileHandle, { } },
        { "_brush", brHandle, { } },
        { "_pipet", piHandle, { } },
        { "_select", seHandle, { } },
    };

    WINDOW *win;
    struct event ev;

    Canvases.p = Malloc(sizeof(*Canvases.p));
    if (Canvases.p == NULL) {
        return -1;
    }
    Canvases.n = 1;

    memset(Canvases.p, 0, sizeof(*Canvases.p));
    Canvases.p[Canvases.sel].data = Newpad(1024, 1024);
    Canvases.p[Canvases.sel].hist.cur = &Canvases.p[Canvases.sel].hist.root;
    Canvases.p[Canvases.sel].hist.root.rgn = CreateRgn(NULL);
    Canvases.p[Canvases.sel].sel = CreateRgn(NULL);

    win = Newpad(1, 1);
    if (win == NULL) {
        return -1;
    }
    waddch(win, '#');
    if (CreateBrush("Default", win, true) == NULL) {
        return -1;
    }

    win = Newpad(3, 3);
    if (win == NULL) {
        return -1;
    }
    mvwaddch(win, 0, 1, '#');
    mvwaddch(win, 1, 1, '#');
    mvwaddch(win, 1, 0, '#');
    mvwaddch(win, 2, 1, '#');
    mvwaddch(win, 1, 2, '#');
    CreateBrush("Thick", win, true);
    /* TODO: NO ERROR CHECKING BECAUSE THIS IS TEMPORARY */

    win = Newpad(1, 1);
    if (win == NULL) {
        return -1;
    }
    waddch(win, ' ');
    if (CreateBrush("Eraser", win, false) == NULL) {
        return -1;
    }

    Toolbar.t = Malloc(sizeof(*Toolbar.t) * ARRAY_SIZE(stools));
    memcpy(Toolbar.t, stools, sizeof(stools));
    Toolbar.n = ARRAY_SIZE(stools);

    Brushes.v = true;

    UIRunning = true;
    while (UIRunning) {
        RenderUI();

        GetEvent(&ev);
        if (ev.type == EV_KEYDOWN && ev.key == 'q') {
            UIRunning = false;
            break;
        }
        cvHandle(&Canvases.p[Canvases.sel], &ev);
        cpHandle(&ev);
        Toolbar.t[Toolbar.sel].handle(&Toolbar.t[Toolbar.sel],
            &Canvases.p[Canvases.sel], &ev);
        tbHandle(&ev);
    }
    return 0;
}

