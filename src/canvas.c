#include "ui.h"
#include "screen.h"
#include "macros.h"

#include <stdlib.h>

struct canvases Canvases;

int UpdateCache(struct canvas *cv)
{
    WINDOW *win, **cache;

    cache = Malloc(sizeof(*cache) * cv->sel->n);
    if (cache == NULL) {
        return -1;
    }
    for (uint32_t i = 0; i < cv->sel->n; i++) {
        Rect *const r = &cv->sel->r[i];
        win = Newpad(r->h, r->w);
        if (win == NULL) {
            return -1;
        }
        cache[i] = win;
        copywin(cv->data, win, r->y, r->x, 0, 0, r->h - 1, r->w - 1, false);
    }

    Free(cv->cache);
    cv->cache = cache;
    return 0;
}

struct hist_event *CreateEvent(struct history *hist,
        enum hist_event_type type, const struct rgn *rgn)
{
    struct hist_event *cur, **c, *hev;

    cur = hist->cur;
    c = Realloc(cur->c, sizeof(*cur->c) * (cur->n + 1));
    if (c == NULL) {
        return NULL;
    }
    cur->c = c;

    hev = Calloc(1, sizeof(*hev));
    if (hev == NULL) {
        return NULL;
    }
    hev->type = type;
    hev->time = time(NULL);
    if (rgn == NULL) {
        hev->rgn = cur->rgn;
    } else {
        hev->rgn = CreateRgn(rgn);
    }

    cur->c[cur->n++] = hev;
    hev->p = cur;
    hist->cur = hev;
    return hev;
}

void DropEvent(struct history *hist)
{
    struct hist_event *hev;

    hev = hist->cur;
    DeleteRgn(hev->rgn);
    hist->cur = hist->cur->p;
    hist->cur->n = 0;
    Free(hev);
}

bool UndoEvent(struct canvas *cv)
{
    struct hist_event *cur;
    struct rgn *dup;

    cur = cv->hist.cur;
    if (cur->p == NULL) {
        return false;
    }
    dup = CreateRgn(cur->p->rgn);
    if (dup == NULL) {
        return false;
    }
    DeleteRgn(cv->sel);
    cv->sel = dup;
    switch (cur->type) {
    case HEV_SELCHG:
        (void) 0;
        break;
    case HEV_SELDEL:
    case HEV_SELPLC:
        for (uint32_t i = 0; i < cv->sel->n; i++) {
            Rect *const r = &cv->sel->r[i];
            copywin(cur->win, cv->data, r->y - cur->dy, r->x - cur->dx,
                    r->y, r->x, r->y + r->h - 1, r->x + r->w - 1, false);
        }
        break;
    case HEV_STROKE:
        copywin(cur->st.cache, cv->data, 0, 0, cur->st.rect.y, cur->st.rect.x,
                cur->st.rect.y + cur->st.rect.h - 1, cur->st.rect.x + cur->st.rect.w - 1,
                false);
        break;
    }
    cv->hist.cur = cur->p;
    return true;
}

bool RedoEvent(struct canvas *cv)
{
    struct hist_event *cur;
    struct rgn *dup;

    cur = cv->hist.cur;
    if (cur->n == 0) {
        return false;
    }
    cur = cur->c[cur->n - 1];
    dup = CreateRgn(cur->rgn);
    if (dup == NULL) {
        return false;
    }
    DeleteRgn(cv->sel);
    cv->sel = dup;
    switch (cur->type) {
    case HEV_SELCHG:
        (void) 0;
        break;
    case HEV_SELDEL:
        wattr_set(cv->data, A_NORMAL, 0, NULL);
        for (uint32_t i = 0; i < cur->rgn->n; i++) {
            Rect *const r = &cur->rgn->r[i];
            for (int y = 0; y < r->h; y++) {
                for (int x = 0; x < r->w; x++) {
                    mvwaddch(cv->data, r->y + y, r->x + x, ' ');
                }
            }
        }
        break;
    case HEV_SELPLC:
        for (uint32_t i = 0; i < cv->sel->n; i++) {
            Rect *const r = &cv->sel->r[i];
            copywin(cur->pwin, cv->data, r->y - cur->dy, r->x - cur->dx,
                    r->y, r->x, r->y + r->h - 1, r->x + r->w - 1, false);
        }
        break;
    case HEV_STROKE:
        DrawStroke(&cur->st, cv->data, 0, 0);
        break;
    }
    cv->hist.cur = cur;
    return true;
}

int cvHandle(struct canvas *cv, struct event *ev)
{
    switch (ev->type) {
    case EV_DRAW:
        copywin(cv->data, stdscr, 0, 0, cv->y, cv->x,
                cv->y + cv->h - 1, cv->x + cv->w - 1, false);
        for (uint32_t i = 0; i < cv->sel->n; i++) {
            Rect *const r = &cv->sel->r[i];
            /*for (int x = 0; x < r->w; x++) {
                addch('$');
            }
            move(cv->y + r->y + r->h - 1, cv->x + r->x);
            for (int x = 0; x < r->w; x++) {
                addch('$');
            }
            for (int y = 1; y < r->h - 1; y++) {
                mvaddch(cv->y + r->y + y,
                        cv->x + r->x, '$');
                mvaddch(cv->y + r->y + y,
                        cv->x + r->x + r->w - 1, '$');
            }*/

            copywin(cv->cache[i], stdscr, 0, 0,
                    r->y + cv->y, r->x + cv->x,
                    r->y + r->h - 1 + cv->y,
                    r->x + r->w - 1 + cv->x, true);
            for (int y = 0; y < r->h; y++) {
                mvchgat(cv->y + r->y + y, cv->x + r->x, r->w, A_REVERSE, 0, NULL);
            }
        }
        break;
    default:
        switch (ev->key) {
        case 'u':
            UndoEvent(cv);
            break;
        case CONTROL('R'):
            RedoEvent(cv);
            break;
        }
    }
    return 0;
}

