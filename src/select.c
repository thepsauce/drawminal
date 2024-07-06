#include "geom.h"
#include "ui.h"

/*
 * implementation of the selection tool for the canvas
 */

static inline void GetSelection(struct tool *tool, Rect *r)
{
    if (tool->sx < tool->ex) {
        r->x = tool->sx;
        r->w = tool->ex - tool->sx + 1;
    } else {
        r->x = tool->ex;
        r->w = tool->sx - tool->ex + 1;
    }
    if (tool->sy < tool->ey) {
        r->y = tool->sy;
        r->h = tool->ey - tool->sy + 1;
    } else {
        r->y = tool->ey;
        r->h = tool->sy - tool->ey + 1;
    }
}

static inline int SetSelectRect(struct canvas *cv, const Rect *r)
{
    struct rgn *rgn;
    struct hist_event *hev;

    rgn = CreateRectRgn(r);
    if (rgn == NULL) {
        return -1;
    }
    hev = CreateEvent(&cv->hist, HEV_SELCHG, rgn);
    if (hev == NULL) {
        DeleteRgn(rgn);
        return -1;
    }
    DeleteRgn(cv->sel);
    cv->sel = rgn;
    return UpdateCache(cv);
}

static inline int AddSelectRect(struct canvas *cv, const Rect *r)
{
    struct rgn *rgn;
    struct hist_event *hev;

    rgn = CreateRgn(cv->sel);
    if (rgn == NULL) {
        return -1;
    }
    if (AddRectRgn(rgn, r) == NULL) {
        DeleteRgn(rgn);
        return -1;
    }
    hev = CreateEvent(&cv->hist, HEV_SELCHG, rgn);
    if (hev == NULL) {
        DeleteRgn(rgn);
        return -1;
    }
    DeleteRgn(cv->sel);
    cv->sel = rgn;
    return UpdateCache(cv);
}

static inline int SubtractSelectRect(struct canvas *cv, const Rect *r)
{
    struct rgn *rgn;
    struct hist_event *hev;

    rgn = CreateRgn(cv->sel);
    if (rgn == NULL) {
        return -1;
    }
    if (SubtractRectRgn(rgn, r) == NULL) {
        DeleteRgn(rgn);
        return -1;
    }
    hev = CreateEvent(&cv->hist, HEV_SELCHG, rgn);
    if (hev == NULL) {
        DeleteRgn(rgn);
        return -1;
    }
    DeleteRgn(cv->sel);
    cv->sel = rgn;
    return UpdateCache(cv);
}

static inline int MoveSelectRgn(struct canvas *cv, int dx, int dy)
{
    struct rgn *rgn;
    struct hist_event *hev;

    rgn = CreateRgn(cv->sel);
    if (rgn == NULL) {
        return -1;
    }
    MoveRgnBy(rgn, dx, dy);
    hev = CreateEvent(&cv->hist, HEV_SELCHG, rgn);
    if (hev == NULL) {
        DeleteRgn(cv->sel);
        return -1;
    }
    DeleteRgn(cv->sel);
    cv->sel = rgn;
    return 0;
}

static inline int InvertSelectRgn(struct canvas *cv)
{
    struct rgn *rgn;
    int mx, my;
    struct hist_event *hev;

    rgn = CreateRgn(NULL);
    if (rgn == NULL) {
        return -1;
    }
    getmaxyx(cv->data, my, mx);
    cv->sel->b = (Rect) { 0, 0, mx, my };
    if (InvertRgn(rgn, cv->sel) == NULL) {
        DeleteRgn(rgn);
        return -1;
    }
    hev = CreateEvent(&cv->hist, HEV_SELCHG, NULL);
    if (hev == NULL) {
        DeleteRgn(rgn);
        return -1;
    }
    DeleteRgn(cv->sel);
    cv->sel = rgn;
    return UpdateCache(cv);
}

static inline int ClearSelectRgn(struct canvas *cv)
{
    struct rgn *rgn;
    struct hist_event *hev;

    rgn = CreateRgn(NULL);
    if (rgn == NULL) {
        return -1;
    }
    hev = CreateEvent(&cv->hist, HEV_SELCHG, rgn);
    if (hev == NULL) {
        DeleteRgn(rgn);
        return -1;
    }
    cv->sel = rgn;
    return UpdateCache(cv);
}

static inline int DeleteSelectedRgn(struct canvas *cv)
{
    struct hist_event *hev;
    WINDOW *win;

    if (cv->sel->n == 0) {
        return 0;
    }

    if (UpdateCache(cv) < 0) {
        return -1;
    }

    hev = CreateEvent(&cv->hist, HEV_SELDEL, NULL);
    if (hev == NULL) {
        return -1;
    }
    win = Newpad(cv->sel->b.h, cv->sel->b.w);
    if (win == NULL) {
        DropEvent(&cv->hist);
        return -1;
    }
    copywin(cv->data, win, cv->sel->b.y, cv->sel->b.x,
            0, 0, cv->sel->b.h - 1, cv->sel->b.w - 1, false);
    wattr_set(cv->data, A_NORMAL, 0, NULL);
    for (uint32_t i = 0; i < cv->sel->n; i++) {
        Rect *const r = &cv->sel->r[i];
        for (int y = 0; y < r->h; y++) {
            for (int x = 0; x < r->w; x++) {
                mvwaddch(cv->data, r->y + y, r->x + x, ' ');
            }
        }
    }
    hev->dx = cv->sel->b.x;
    hev->dy = cv->sel->b.y;
    hev->win = win;
    return 0;
}

static inline int PlaceSelectedRgn(struct canvas *cv)
{
    struct hist_event *hev;
    WINDOW *win, *win2;

    if (cv->sel->n == 0) {
        return 0;
    }

    hev = CreateEvent(&cv->hist, HEV_SELPLC, NULL);
    if (hev == NULL) {
        return -1;
    }
    win = Newpad(cv->sel->b.h, cv->sel->b.w);
    if (win == NULL) {
        DropEvent(&cv->hist);
        return -1;
    }
    win2 = Newpad(cv->sel->b.h, cv->sel->b.w);
    if (win2 == NULL) {
        delwin(win);
        DropEvent(&cv->hist);
        return -1;
    }
    copywin(cv->data, win, cv->sel->b.y, cv->sel->b.x,
            0, 0, cv->sel->b.h - 1, cv->sel->b.w - 1, false);
    for (uint32_t i = 0; i < cv->sel->n; i++) {
        Rect *const r = &cv->sel->r[i];
        copywin(cv->cache[i], win2, 0, 0,
                r->y - cv->sel->b.y,
                r->x - cv->sel->b.x,
                r->y - cv->sel->b.y + r->h - 1,
                r->x - cv->sel->b.x + r->w - 1, true);
        copywin(cv->cache[i], cv->data, 0, 0, r->y, r->x,
            r->y + r->h - 1, r->x + r->w - 1, true);
    }
    hev->dx = cv->sel->b.x;
    hev->dy = cv->sel->b.y;
    hev->win = win;
    hev->pwin = win2;
    return 0;
}

int seHandle(struct tool *tool, struct canvas *cv, struct event *ev)
{
    Rect r, sel;

    GetSelection(tool, &sel);
    switch (ev->type) {
    case EV_DRAW:
        r = (Rect) { Toolbar.x, Toolbar.y, Toolbar.w, Toolbar.h };
        DrawString(stdscr, &r, DS_DSEQ,
                "$rA$0ll $ra$0$append$0 $rc$0lear $rd$0elete "
                "$ri$0nvert $rs$0$aubtract$0 Place $r<Return>$0",
                tool->am ? A_BOLD : 0, tool->sm ? A_BOLD : 0);

        if (tool->sel) {
            for (int x = 0; x < sel.w; x++) {
                mvaddch(cv->y + sel.y, cv->x + sel.x + x, '-');
                mvaddch(cv->y + sel.y + sel.h - 1, cv->x + sel.x + x, '-');
            }
            for (int y = 1; y < sel.h - 1; y++) {
                mvaddch(cv->y + sel.y + y, cv->x + sel.x, '|');
                mvaddch(cv->y + sel.y + y, cv->x + sel.x + sel.w - 1, '|');
            }
        }
        break;

    case EV_LBUTTONDOWN:
        if (tool->move) {
            tool->move = false;
            MoveRgnBy(cv->sel, -tool->dx, -tool->dy);
            break;
        }
        tool->sel = true;
        tool->sx = Mouse.x - cv->x;
        tool->sy = Mouse.y - cv->y;
        /* fall through */
    case EV_MOUSEMOVE:
        if (Mouse.bt[BT_LEFT] && tool->sel) {
            tool->ex = Mouse.x - cv->x;
            tool->ey = Mouse.y - cv->y;
        } else if (Mouse.bt[BT_RIGHT] && tool->move) {
            tool->dx += Mouse.x - Mouse.px;
            tool->dy += Mouse.y - Mouse.py;
            MoveRgnBy(cv->sel, Mouse.x - Mouse.px, Mouse.y - Mouse.py);
        }
        break;

    case EV_RBUTTONDOWN:
        if (tool->sel) {
            tool->sel = false;
            break;
        }
        tool->move = true;
        tool->dx = 0;
        tool->dy = 0;
        break;

    case EV_LBUTTONUP:
        if (!tool->sel) {
            break;
        }
        tool->sel = false;
        if (tool->sm) {
            return SubtractSelectRect(cv, &sel);
        } else if (tool->am) {
            return AddSelectRect(cv, &sel);
        } else {
            return SetSelectRect(cv, &sel);
        }
        break;

    case EV_RBUTTONUP:
        if (!tool->move) {
            break;
        }
        tool->move = false;
        MoveRgnBy(cv->sel, -tool->dx, -tool->dy);
        return MoveSelectRgn(cv, tool->dx, tool->dy);

    default:
        switch (ev->key) {
        case '\n':
            return PlaceSelectedRgn(cv);
        case 'A':
            r = (Rect) { 0, 0, cv->w, cv->h };
            return SetSelectRect(cv, &r);
        case 'a':
            tool->am = !tool->am;
            if (tool->am) {
                tool->sm = false;
            }
            break;
        case 'c':
            DeleteRgn(cv->sel);
            return ClearSelectRgn(cv);

        case 'd':
        case KEY_DC:
            return DeleteSelectedRgn(cv);

        case 'i':
            return InvertSelectRgn(cv);

        case 's':
            tool->sm = !tool->sm;
            if (tool->sm) {
                tool->am = false;
            }
            break;
        }
    }
    return 0;
}
