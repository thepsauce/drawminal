#include "screen.h"
#include "macros.h"
#include "geom.h"

#include <stdlib.h>
#include <string.h>

struct rgn *CreateRgn(const struct rgn *dup)
{
    struct rgn *rgn;

    rgn = Malloc(sizeof(*rgn));
    if (rgn == NULL) {
        return NULL;
    }
    if (dup == NULL || dup->n == 0) {
        rgn->r = NULL;
        rgn->n = 0;
        rgn->b = (Rect) { 0, 0, 0, 0 };
    } else {
        rgn->r = Malloc(sizeof(*rgn->r) * dup->n);
        if (rgn->r == NULL) {
            Free(rgn);
            return NULL;
        }
        memcpy(rgn->r, dup->r, sizeof(*rgn->r) * dup->n);
        rgn->n = dup->n;
        rgn->b = dup->b;
    }
    return rgn;
}

struct rgn *SetRgnEmpty(struct rgn *rgn)
{
    if (rgn->r != NULL) {
        Free(rgn->r);
        rgn->r = NULL;
    }
    rgn->n = 0;
    rgn->b = (Rect) { 0, 0, 0, 0 };
    return rgn;
}

struct rgn *CreateRectRgn(const Rect *rect)
{
    struct rgn *rgn;

    rgn = CreateRgn(NULL);
    if (rgn == NULL) {
        return NULL;
    }
    rgn->r = Malloc(sizeof(*rgn->r));
    if (rgn->r == NULL) {
        Free(rgn);
        return NULL;
    }
    rgn->r[0] = *rect;
    rgn->n = 1;
    rgn->b = *rect;
    return rgn;
}

struct rgn *SetRectRgn(struct rgn *rgn, const Rect *rect)
{
    Rect *p;

    if (IsRectEmpty(rect)) {
        return SetRgnEmpty(rgn);
    }

    p = Realloc(rgn->r, sizeof(*rgn->r));
    if (p == NULL) {
        return NULL;
    }
    rgn->r = p;
    rgn->r[0] = *rect;
    rgn->n = 1;
    rgn->b = *rect;
    return rgn;
}

struct rgn *AddRectRgn(struct rgn *rgn, const Rect *rect)
{
    Rect *p;
    int32_t dx1, dx2, dy1, dy2;

    p = Realloc(rgn->r, sizeof(*rgn->r) * (rgn->n + 1));
    if (p == NULL) {
        return NULL;
    }
    rgn->r = p;
    rgn->r[rgn->n++] = *rect;

    dx1 = rect->x - rgn->b.x;
    dx2 = rgn->b.x + rgn->b.w - (rect->x + rect->w);
    dy1 = rect->y - rgn->b.y;
    dy2 = rgn->b.y + rgn->b.h - (rect->y + rect->h);
    if (dx1 < 0) {
        rgn->b.x += dx1;
        rgn->b.w -= dx1;
    }
    if (dx2 < 0) {
        rgn->b.w -= dx2;
    }
    if (dy1 < 0) {
        rgn->b.y += dy1;
        rgn->b.h -= dy1;
    }
    if (dy2 < 0) {
        rgn->b.h -= dy2;
    }
    return rgn;
}

static void CompactRgnBounds(struct rgn *rgn)
{
    Rect r = { 0, 0, 0, 0 };

    for (uint32_t i = 0; i < rgn->n; i++) {
        Rect *const r2 = &rgn->r[i];
        r.x = MIN(r.x, r2->x);
        r.y = MIN(r.y, r2->y);
        r.w = MAX(r.x + r.w, r2->x + r2->w);
        r.h = MAX(r.y + r.h, r2->y + r2->h);
    }
    r.w -= r.x;
    r.h -= r.y;
    rgn->b = r;
}

struct rgn *SubtractRectRgn(struct rgn *rgn, const Rect *rect)
{
    Rect rects[4];
    Rect *next, *p;
    uint32_t n = 0, c;

    c = rgn->n;
    next = Malloc(sizeof(*next) * c);
    if (next == NULL) {
        return NULL;
    }
    for (uint32_t i = 0; i < rgn->n; i++) {
        Rect *const r = &rgn->r[i];
        const int s = CutRect(r, rect, rects);
        if (s > 0) {
            if (n + s > c) {
                c *= 2;
                c += s;
                p = Realloc(next, sizeof(*next) * c);
                if (p == NULL) {
                    Free(next);
                    return NULL;
                }
                next = p;
            }
            memcpy(&next[n], rects, sizeof(*rects) * s);
            n += s;
        }
    }
    next = Realloc(next, sizeof(*next) * n);
    if (next == NULL) {
        return NULL;
    }
    Free(rgn->r);
    rgn->r = next;
    rgn->n = n;
    CompactRgnBounds(rgn);
    return rgn;
}

struct rgn *IntersectRgn(struct rgn *rgn, const struct rgn *rgn1, const struct rgn *rgn2)
{
    if (rgn1 == rgn2) {
        if (rgn != rgn1) {
            if (rgn1->n == 0) {
                SetRgnEmpty(rgn);
                return rgn;
            }
            Rect *const r = Malloc(sizeof(*r) * rgn1->n);
            if (r == NULL) {
                return NULL;
            }
            memcpy(r, rgn1->r, sizeof(*r) * rgn1->n);
            rgn->r = r;
            rgn->n = rgn1->n;
            rgn->b = rgn1->b;
        }
        return rgn;
    }

    const uint32_t s = rgn->n;
    for (uint32_t i = 0, ni = rgn1->n, nj = rgn2->n; i < ni; i++) {
        for (uint32_t j = 0; j < nj; j++) {
            Rect r;

            if (IntersectRect(&rgn1->r[i], &rgn2->r[j], &r)) {
                if (AddRectRgn(rgn, &r) == NULL) {
                    rgn->n = s;
                    return NULL;
                }
            }
        }
    }
    rgn->n -= s;
    memmove(&rgn->r[0], &rgn->r[s], sizeof(*rgn->r) * rgn->n);
    CompactRgnBounds(rgn);
    return rgn;
}

struct rgn *AddRgn(struct rgn *rgn, const struct rgn *rgn1, const struct rgn *rgn2)
{
    uint32_t n;
    Rect *p;

    n = rgn1->n + rgn2->n;
    if (n == 0) {
        SetRgnEmpty(rgn);
        return rgn;
    }
    p = Realloc(rgn->r, sizeof(*rgn->r) * n);
    if (p == NULL) {
        return NULL;
    }
    rgn->r = p;

    memcpy(rgn->r, rgn1->r, sizeof(*rgn1->r) * rgn1->n);
    memcpy(&rgn->r[rgn1->n], rgn2->r, sizeof(*rgn2->r) * rgn2->n);
    rgn->n = n;

    RectUnion(&rgn1->b, &rgn2->b, &rgn->b);
    return rgn;
}

static int CompareRectsX(const void *a, const void *b)
{
    const Rect *const r1 = a, *const r2 = b;
    return r1->x - r2->x;
}

static int CompareRectsY(const void *a, const void *b)
{
    const Rect *const r1 = a, *const r2 = b;
    return r1->y - r2->y;
}

struct rgn *InvertRgn(struct rgn *rgn, const struct rgn *rgn1)
{
    Rect r;
    uint32_t ys, yi;

    rgn->b = rgn1->b;

    /* sorting at the beginning allows for smaller lookup sizes */
    qsort((Rect*) rgn1->r, rgn1->n, sizeof(*rgn1->r), CompareRectsY);
    ys = 0;
    yi = 0;
    r.y = rgn1->b.y;
    r.h = 0;
    const uint32_t n = rgn1->n, s = rgn->n;
    for (int32_t i = 0; i < rgn1->b.h; i++) {
        for (; ys < n; ys++) {
            if (rgn1->r[ys].y + rgn1->r[ys].h > r.y) {
                break;
            }
        }

        for (yi = ys; yi < n; yi++) {
            if (rgn1->r[yi].y > r.y + r.h) {
                break;
            }
        }

        if (yi == ys) {
            r.h++;
        } else {
            Rect rx[yi - ys + 1];

            /* there exist some rectangles interfering with the scanline */
            if (r.h > 0) {
                r.x = rgn1->b.x;
                r.w = rgn1->b.w;
                if (AddRectRgn(rgn, &r) == NULL) {
                    rgn->n = s;
                    return NULL;
                }
                r.y += r.h;
            }
            memcpy(rx, &rgn1->r[ys], sizeof(*rx) * (yi - ys));
            qsort(rx, yi - ys, sizeof(*rx), CompareRectsX);
            rx[yi - ys] = (Rect) { rgn1->b.x + rgn1->b.w, r.y, 0, 1 };
            r.x = rgn1->b.x;
            for (uint32_t xi = 0; xi <= yi - ys; xi++) {
                if (rx[xi].y + rx[xi].h <= r.y ||
                        rx[xi].x + rx[xi].w <= r.x) {
                    continue;
                }
                r.w = rx[xi].x - r.x;
                if (r.w > 0) {
                    uint32_t j;

                    /* join rects that align perfectly */
                    for (j = s; j < rgn->n; j++) {
                        const Rect ro = rgn->r[j];
                        if (ro.y + ro.h != r.y) {
                            continue;
                        }
                        if (ro.x == r.x && ro.w == r.w) {
                            rgn->r[j].h++;
                            break;
                        }
                    }
                    if (j == rgn->n) {
                        r.h = 1;
                        if (AddRectRgn(rgn, &r) == NULL) {
                            rgn->n = s;
                            return NULL;
                        }
                    }
                }
                r.x = rx[xi].x + rx[xi].w;
            }
            r.y++;
            r.h = 0;
        }
    }
    if (r.h > 0) {
        r.x = rgn1->b.x;
        r.w = rgn1->b.w;
        if (AddRectRgn(rgn, &r) == NULL) {
            rgn->n = s;
            return NULL;
        }
    }
    rgn->n -= s;
    memmove(&rgn->r[0], &rgn->r[s], sizeof(*rgn->r) * rgn->n);
    CompactRgnBounds(rgn);
    return rgn;
}

struct rgn *SubtractRgn(struct rgn *rgn, const struct rgn *rgn1, const struct rgn *rgn2)
{
    struct rgn *tmp;

    const Rect ob = rgn2->b;
    RectUnion(&rgn1->b, &rgn2->b, (Rect*) &rgn2->b);
    tmp = CreateRgn(NULL);
    InvertRgn(tmp, rgn2);
    IntersectRgn(rgn, tmp, rgn1);
    DeleteRgn(tmp);
    *((Rect*) &rgn2->b) = ob;
    return rgn;
}

struct rgn *MoveRgnBy(struct rgn *rgn, int32_t dx, int32_t dy)
{
    for (uint32_t i = 0; i < rgn->n; i++) {
        rgn->r[i].x += dx;
        rgn->r[i].y += dy;
    }
    rgn->b.x += dx;
    rgn->b.y += dy;
    return rgn;
}

void DeleteRgn(struct rgn *rgn)
{
    Free(rgn->r);
    Free(rgn);
}
