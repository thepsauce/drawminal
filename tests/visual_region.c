#include "screen.h"

#include <time.h>

static inline void GetSelection(int *psx, int *psy, int *pex, int *pey)
{
    const int sx = *psx;
    const int sy = *psy;
    const int ex = *pex;
    const int ey = *pey;
    if (sx < ex) {
        *psx = sx;
        *pex = ex;
    } else {
        *psx = ex;
        *pex = sx;
    }
    if (sy < ey) {
        *psy = sy;
        *pey = ey;
    } else {
        *psy = ey;
        *pey = sy;
    }
}

int main(void)
{
    bool r = true;
    struct event ev;
    int sx = 0, sy = 0, ex = 0, ey = 0;
    int tsx, tsy, tex, tey;
    bool sel = false;
    Rect s;
    enum {
        M_APPEND,
        M_SUBTRACT,
        M_SET,
    } mode = M_APPEND;

    struct rgn *rgn, *tmp;

    InitScreen();

    rgn = CreateRgn();
    if (rgn == NULL) {
        return -1;
    }

    while (r) {
        tsx = sx;
        tsy = sy;
        tex = ex;
        tey = ey;

        erase();

        char b[128];
        sprintf(b, "%ld", time(NULL));
        mvaddstr(0, 0, b);

        for (uint32_t i = 0; i < rgn->n; i++) {
            Rect *const r = &rgn->r[i];
            for (int y = 0; y < r->h; y++) {
                mvchgat(r->y + y, r->x, r->w, A_REVERSE, 0, NULL);
            }
        }

        if (sel) {
            GetSelection(&tsx, &tsy, &tex, &tey);
            s = (Rect) { tsx, tsy, tex - tsx + 1, tey - tsy + 1 };
            DrawBox(NULL, &s);
        }

        GetEvent(&ev);
        switch (ev.type) {
        case EV_LBUTTONDOWN:
            sel = true;
            sx = Mouse.x;
            sy = Mouse.y;
            /* fall through */
        case EV_MOUSEMOVE:
            if (Mouse.bt[BT_LEFT]) {
                ex = Mouse.x;
                ey = Mouse.y;
            }
            break;
        case EV_LBUTTONUP:
            if (!sel) {
                break;
            }
            tmp = RectRgn(&s);
            switch (mode) {
            case M_APPEND:
                AddRgn(rgn, rgn, tmp);
                break;
            case M_SUBTRACT:
                SubtractRgn(rgn, rgn, tmp);
                break;
            case M_SET:
                SetRectRgn(rgn, &s);
                break;
            }
            DeleteRgn(tmp);
            sel = false;
            break;
        case EV_KEYDOWN:
            switch (ev.key) {
            case 'a':
                mode = M_APPEND;
                break;
            case 's':
                mode = M_SUBTRACT;
                break;
            case 't':
                mode = M_SET;
                break;
            case 'i':
                rgn->b = (Rect) { 0, 0, COLS, LINES };
                InvertRgn(rgn, rgn);
                break;
            case 'n':
                IntersectRgn(rgn, rgn, rgn);
                break;
            case 'A':
                SetRectRgn(rgn, &(Rect) { 0, 0, COLS, LINES });
                break;
            case 'q':
                r = false;
                break;
            }
        default:
        }
    }

    EndScreen();
    return 0;
}
