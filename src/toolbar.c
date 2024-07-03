#include "screen.h"
#include "ui.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

struct toolbar Toolbar;

static void DrawTip(const char *tip, bool high)
{
    for (; *tip != '\0'; tip++) {
        if (!high) {
            if (*tip == '_') {
                tip++;
                attr_set(A_NORMAL, CP_RED, NULL);
            } else {
                attr_set(A_NORMAL, 0, NULL);
            }
        } else {
            if (*tip == '_') {
                tip++;
            }
            attr_set(A_BOLD, 0, NULL);
        }
        addch(*tip);
    }
}

void tbHandle(struct event *ev)
{
    int x = 0;

    switch (ev->type) {
    case EV_DRAW:
        move(Toolbar.y + 1, Toolbar.x);
        for (unsigned i = 0; i < Toolbar.n; i++) {
            DrawTip(Toolbar.t[i].name, i == Toolbar.sel);
            addch(' ');
        }
        break;

    case EV_LBUTTONDOWN:
        if (Mouse.y != Toolbar.y + Toolbar.h - 1) {
            break;
        }
        for (unsigned i = 0; i < Toolbar.n; i++) {
            const int n = (int) strlen(Toolbar.t[i].name);
            if (Mouse.x - Toolbar.x <= x + n) {
                Toolbar.sel = i;
                break;
            }
            x += n;
        }
        break;

    case EV_KEYDOWN:
        for (unsigned i = 0; i < Toolbar.n; i++) {
            for (const char *s = Toolbar.t[i].name; *s != '\0'; s++) {
                if (*s == '_') {
                    if (*s == '\0') {
                        break;
                    }
                    s++;
                    if (*s == ev->key) {
                        Toolbar.sel = i;
                        return;
                    }
                }
            }
        }
        break;
    default:
        break;
    }
}
