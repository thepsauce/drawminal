#include <limits.h>
#include <string.h>

#include "macros.h"
#include "screen.h"

void DrawBrushHighlight2(WINDOW *win, chtype ch, int x, int y, int d)
{
    int r;
    int xi, yi;
    int p;

    r = d >> 1;
    mvwaddch(win, y, x + r, ch);
    if (r == 0) {
        return;
    }

    mvwaddch(win, y - r, x, ch);
    mvwaddch(win, y + r, x, ch);
    mvwaddch(win, y, x - r, ch);
    xi = r;
    yi = 0;
    p = 1 - r;
    while (xi > yi) {
        yi++;
        if (p <= 0) {
            p += 2 * yi + 1;
        } else {
            xi--;
            p += 2 * (yi - xi) + 1;
        }

        if (xi < yi) {
            break;
        }

        mvwaddch(win, y + yi, x + xi, ch);
        mvwaddch(win, y + yi, x - xi, ch);
        mvwaddch(win, y - yi, x + xi, ch);
        mvwaddch(win, y - yi, x - xi, ch);

        if (xi != yi) {
            mvwaddch(win, y + xi, x + yi, ch);
            mvwaddch(win, y + xi, x - yi, ch);
            mvwaddch(win, y - xi, x + yi, ch);
            mvwaddch(win, y - xi, x - yi, ch);
        }
    }
}

void DrawBrushPattern2(WINDOW *win, chtype ch, int x, int y, int d)
{
    int r;
    int xi, yi;
    int p;

    r = d >> 1;
    if (r == 0) {
        mvwaddch(win, y, x + r, ch);
        return;
    }

    mvwhline(win, y, x - r, ch, 2 * r + 1);
    xi = r;
    yi = 0;
    p = 1 - r;
    while (xi > yi) {
        yi++;
        if (p <= 0) {
            p += 2 * yi + 1;
        } else {
            xi--;
            p += 2 * (yi - xi) + 1;
        }

        if (xi < yi) {
            break;
        }

        mvwhline(win, y - yi, x - xi, ch, 2 * xi + 1);
        mvwhline(win, y + yi, x - xi, ch, 2 * xi + 1);

        if (xi != yi) {
            mvwhline(win, y - xi, x - yi, ch, 2 * yi + 1);
            mvwhline(win, y + xi, x - yi, ch, 2 * yi + 1);
        }
    }
}

void DrawLine2(WINDOW *win, chtype ch, int width, int x1, int y1, int x2, int y2)
{
    int err, err2;
    int dx, dy, sx, sy;

    if (x1 < x2) {
        dx = x2 - x1;
        sx = 1;
    } else {
        dx = x1 - x2;
        sx = -1;
    }

    if (y1 < y2) {
        dy = y1 - y2;
        sy = 1;
    } else {
        dy = y2 - y1;
        sy = -1;
    }

    err = dx + dy;
    while (DrawBrushPattern2(win, ch, x1, y1, width), x1 != x2 || y1 != y2) {
        err2 = 2 * err;
        if (err2 >= dy) /* e_xy + e_x > 0 */ {
            err += dy;
            x1 += sx;
        }
        if (err2 <= dx) /* e_xy + e_y < 0 */ {
            err += dx;
            y1 += sy;
        }
    }
}

int main(void)
{
    struct brush {
        const char *name;
        chtype ch;
        int highlight;
        int width;
    } brushes[2] = {
        { "Brush", '#', 0, 1 },
        { "Eraser", ' ', 0, 8 }
    };
    int curColor = 7;
    int curBrush = 0;
    bool isPressed = false;
    int prevX = 0, prevY = 0, curX = INT_MIN, curY = INT_MIN;
    MEVENT m;
    WINDOW *canvas;

    InitScreen();

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);

    for (int i = 0; i < 8; i++) {
        init_pair(8 + i, i, COLOR_BLACK);
    }

    /* just be a large canvas */
    canvas = Newpad(1024, 1024);
    if (canvas == NULL) {
        return -1;
    }

    while (1) {
        erase();
        attr_set(A_NORMAL, 1, NULL);
        copywin(canvas, stdscr, 0, 0, 1, 0, LINES - 2, COLS - 1, false);
        if (curY >= 1 && curY < LINES - 1) {
            DrawBrushHighlight2(stdscr, '*', curX, curY,
                    brushes[curBrush].width);
        }
        move(0, 0);
        for (int i = 0; i < 8; i++) {
            attr_set(A_REVERSE, 8 + i, NULL);
            addch(' ');
            addch(' ');
        }
        move(LINES - 1, 0);
        for (int i = 0; i < (int) ARRAY_SIZE(brushes); i++) {
            int x, y, nx, ny;

            if (i == curBrush) {
                attr_set(A_BOLD, 0, NULL);
            } else {
                attr_set(A_NORMAL, 0, NULL);
            }
            getyx(stdscr, y, x);
            addstr(brushes[i].name);
            getyx(stdscr, ny, nx);
            if (i != curBrush) {
                move(y, x + brushes[i].highlight);
                attr_set(A_NORMAL, 2, NULL);
                addch(brushes[i].name[brushes[i].highlight]);
                move(ny, nx);
            }
            addch(' ');
        }
        const int c = getch();
        if (c == 'q') {
            break;
        }
        switch (c) {
            case 'b':
                curBrush = 0;
                break;
            case 'e':
                curBrush = 1;
                break;
            case '+':
                if (brushes[curBrush].width == 32) {
                    break;
                }
                brushes[curBrush].width++;
                break;
            case '-':
                if (brushes[curBrush].width == 1) {
                    break;
                }
                brushes[curBrush].width--;
                break;
            case KEY_MOUSE:
                if (getmouse(&m) == OK) {
                    curX = m.x;
                    curY = m.y;
                    if (m.bstate == BUTTON4_PRESSED) {
                        if (brushes[curBrush].width == 32) {
                            break;
                        }
                        brushes[curBrush].width++;
                        break;
                    }
                    if (m.bstate == BUTTON5_PRESSED) {
                        if (brushes[curBrush].width == 1) {
                            break;
                        }
                        brushes[curBrush].width--;
                        break;
                    }
                    if (m.bstate == BUTTON1_PRESSED) {
                        prevX = m.x;
                        prevY = m.y;
                        isPressed = true;
                    } else if (m.bstate == BUTTON1_RELEASED) {
                        isPressed = false;
                    }
                    if (m.y == 0) {
                        if (m.bstate == BUTTON1_PRESSED) {
                            curColor = m.x / 2;
                            if (curColor >= 8) {
                                curColor = 7;
                            }
                        }
                        break;
                    } else if (m.y == LINES - 1) {
                        if (m.bstate == BUTTON1_PRESSED) {
                            int x = 0;

                            for (int i = 0; i < (int) ARRAY_SIZE(brushes); i++) {
                                if (m.x <= (int) (x + strlen(brushes[i].name))) {
                                    curBrush = i;
                                    break;
                                }
                                x += strlen(brushes[i].name);
                            }
                        }
                        break;
                    }
                    if (isPressed) {
                        wattr_set(canvas, A_NORMAL, curColor + 8, NULL);
                        DrawLine2(canvas,
                            brushes[curBrush].ch, brushes[curBrush].width,
                            prevX, prevY - 1, m.x, m.y - 1);
                        prevX = m.x;
                        prevY = m.y;
                    }
                }
        }
    }

    EndScreen();
    return 0;
}
