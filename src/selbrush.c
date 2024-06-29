#include "macros.h"
#include "ui.h"
#include "screen.h"

int sbrHandle(struct event *ev)
{
    int y;
    int mx, my;

    if (!Brushes.v) {
        return 0;
    }

    switch (ev->type) {
    case EV_DRAW:
        attr_set(A_NORMAL, 0, NULL);
        mvvline(Brushes.y, Brushes.x, '|', Brushes.h);
        attr_set(A_NORMAL, CP_RED, NULL);
        mvaddch(Brushes.y, Brushes.x, '^');
        mvaddch(Brushes.y + 1, Brushes.x, 'k');
        mvaddch(Brushes.y + 2, Brushes.x, ' ');
        mvaddch(Brushes.y + 3, Brushes.x, 'j');
        mvaddch(Brushes.y + 4, Brushes.x, 'v');
        y = Brushes.y;
        for (unsigned i = 0; i < Brushes.n; i++) {
            struct brush *const br = Brushes.p[i];
            getmaxyx(br->pat, my, mx);
            if (i == Brushes.sel) {
                attr_set(A_REVERSE, 0, NULL);
            } else {
                attr_set(A_NORMAL, 0, NULL);
            }
            mvaddstr(y + my / 2, Brushes.x + 1, br->name);
            const int x = Brushes.x + (Brushes.w - mx) / 2;
            copywin(br->pat, stdscr, 0, 0, y, x, y + my - 1, x + mx - 1, false);
            y += my;
        }
        break;

    case EV_LBUTTONDOWN:
        if (!INSIDE_RECT(Brushes, Mouse)) {
            break;
        }
        y = Brushes.y;
        for (unsigned i = 0; i < Brushes.n; i++) {
            getmaxyx(Brushes.p[i]->pat, my, mx);
            y += my;
            if (Mouse.y <= y) {
                Brushes.v = false;
                Brushes.sel = i;
                break;
            }
        }
        break;

    case EV_KEYDOWN:
        switch (ev->key) {
        case 'j':
            if (Brushes.sel + 1 == Brushes.n) {
                Brushes.sel = 0;
            } else {
                Brushes.sel++;
            }
            break;
        case 'k':
            if (Brushes.sel == 0) {
                Brushes.sel = Brushes.n - 1;
            } else {
                Brushes.sel--;
            }
            break;
        case '+':
            break;
        case '-':
            break;
        }
        break;
    default:
    }
    return 0;
}
