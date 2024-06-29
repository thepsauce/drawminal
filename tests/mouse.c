#include "screen.h"

int main(void)
{
    InitScreen();

    while (1) {
        static bool isPressed[3];
        const int c = getch();
        if (c == 'q') {
            break;
        }
        if (c == KEY_MOUSE) {
            MEVENT m;

            if (getmouse(&m) == OK) {
                if (m.bstate == BUTTON1_PRESSED) {
                    isPressed[0] = true;
                } else if (m.bstate == BUTTON1_RELEASED) {
                    isPressed[0] = false;
                }
                if (m.bstate == BUTTON2_PRESSED) {
                    isPressed[1] = true;
                } else if (m.bstate == BUTTON2_RELEASED) {
                    isPressed[1] = false;
                }
                if (m.bstate == BUTTON3_PRESSED) {
                    isPressed[2] = true;
                } else if (m.bstate == BUTTON3_RELEASED) {
                    isPressed[2] = false;
                }
                mvprintw(0, 0, "MOUSE: %d, %d, %#x", m.x, m.y, m.bstate);
                clrtoeol();
                move(1, 0);
                for (int i = 0; i < 3; i++) {
                    if (isPressed[i]) {
                        addch('#');
                    } else {
                        addch(' ');
                    }
                }
                clrtoeol();
            }
        } else {
            mvprintw(2, 0, "%s", keyname(c));
            clrtoeol();
        }
    }

    EndScreen();
    return 0;
}
