#include "matrix.h"
#include "screen.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

bool isWithinWindow(WINDOW *win, int y, int x) {
    int winStartY, winStartX, winHeight, winWidth;
    getbegyx(win, winStartY, winStartX);
    getmaxyx(win, winHeight, winWidth);
    return (
        y >= winStartY && y < winStartY + winHeight && x >= winStartX &&
        x < winStartX + winWidth);
}

bool isPointOccupied(WINDOW *canvas, int y, int x) {
    chtype ch = mvwinch(canvas, y, x);
    return (ch != ' ');
}

Matrix *getStateMatrix(WINDOW *canvas) {
    int canvasHeight, canvasWidth;
    getmaxyx(canvas, canvasHeight, canvasWidth);

    Matrix *state = matCreate(canvasHeight, canvasWidth);

    for (int row = 1; row <= canvasHeight; row++) {
        for (int col = 1; col <= canvasWidth; col++) {
            state->elements[ELE_POS(state, row, col)] =
                (int)mvwinch(canvas, row, col);
        }
    }

    // for (size_t i = 0; i < canvasWidth * canvasHeight; i++) {
    //     printf("%f\t", state->elements[i]);
    // }
    return state;
}

void fillCanvas(WINDOW *canvas, Matrix *m) {
    wclear(canvas);
    wrefresh(canvas);
    int canvasHeight, canvasWidth;
    getmaxyx(canvas, canvasHeight, canvasWidth);

    int arraySize = canvasHeight * canvasWidth;
    if (arraySize != m->rows * m->cols) {
        Panic("Invalid occupancy array size");
        return;
    }

    for (int y = 0; y < canvasHeight; y++) {
        for (int x = 0; x < canvasWidth; x++) {
            int index = y * canvasWidth + x;
            mvwaddch(canvas, y, x, (char)m->elements[index]);
        }
    }
    wrefresh(canvas);
}

void canvasFillChar(WINDOW *canvas, char c) {
    int canvasHeight, canvasWidth;
    getmaxyx(canvas, canvasHeight, canvasWidth);

    Matrix *m = matCreate(canvasHeight, canvasWidth);
    matFill(m, (int)c);

    fillCanvas(canvas, m);
}

void canvasClear(WINDOW *canvas) {
    int canvasHeight, canvasWidth;
    getmaxyx(canvas, canvasHeight, canvasWidth);

    Matrix *m = matCreate(canvasHeight, canvasWidth);
    matFill(m, 32);

    fillCanvas(canvas, m);
}

Vec2 getCanvasMidpoint(WINDOW *canvas) {
    int canvasHeight, canvasWidth;
    getmaxyx(canvas, canvasHeight, canvasWidth);
    return (Vec2){round(canvasHeight / 2), round(canvasWidth / 2)};
}

void canvasRotate(WINDOW *canvas, float theta) {
    Matrix *currentState = getStateMatrix(canvas);
    printf(
        "%i, %i %f\n", currentState->rows, currentState->cols,
        currentState->elements[45]);
    Matrix *newState =
        rotateCoordMatrix(currentState, getCanvasMidpoint(canvas), theta);
    fillCanvas(canvas, newState);
}

void DrawBrushHighlight2(WINDOW *win, chtype ch, int x, int y, int d) {
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

int main(void) {
    InitScreen();

    // Canvas dimensions and positioning
    int canvasWidth = 50;
    int canvasHeight = 20;
    int canvasStartY = (LINES - canvasHeight) / 2;
    int canvasStartX = (COLS - canvasWidth) / 2;

    WINDOW *canvas = Newpad(canvasHeight, canvasWidth);
    if (canvas == NULL) {
        return -1;
    }

    bool drawMode = false;
    while (1) {
        erase();
        copywin(
            canvas, stdscr, 0, 0, canvasStartY, canvasStartX,
            canvasStartY + canvasHeight - 1, canvasStartX + canvasWidth - 1,
            false);
        wrefresh(canvas);
        refresh();
        box(canvas, 0, 0);

        MEVENT event;
        int ch = getch();

        switch (ch) {
            case 'Q': {
                EndScreen();
                return 0;
            } break;
            case 'H': {
                canvasRotate(canvas, M_PI / 2);
            } break;
            case 'S': {
                canvasFillChar(canvas, ';');
            } break;
            case 'C': {
                canvasClear(canvas);
            } break;
            case KEY_MOUSE: {
                if (event.bstate & BUTTON1_PRESSED) {
                    drawMode = true;
                }
                if (event.bstate & BUTTON1_RELEASED) {
                    drawMode = false;
                }
                if (getmouse(&event) == OK && drawMode) {
                    if (event.y >= canvasStartY &&
                        event.y < canvasStartY + canvasHeight &&
                        event.x >= canvasStartX &&
                        event.x < canvasStartX + canvasWidth) {
                        DrawBrushHighlight2(
                            canvas, 'a', event.x - canvasStartX,
                            event.y - canvasStartY, 1);
                    }
                }
            } break;
            default:
                continue;
        }
    }
    return 0;
}

