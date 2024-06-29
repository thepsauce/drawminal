#include "macros.h"
#include "ui.h"

#include <string.h>

struct colorpicker ColorPicker;

void cpHandle(struct event *ev)
{
    int color;

    switch (ev->type) {
    case EV_DRAW:
        move(ColorPicker.y, ColorPicker.x);
        for (int i = 0; i < 8; i++) {
            attr_set(A_REVERSE, i, NULL);
            addch(' ');
            addch(' ');
        }
        break;
    case EV_LBUTTONDOWN:
        if (!INSIDE_RECT(ColorPicker, Mouse)) {
            break;
        }
        color = MIN(Mouse.x / 2, 7);
        SetBrushColor(Brushes.p[Brushes.sel], color);
        break;
    default:
    }
}
