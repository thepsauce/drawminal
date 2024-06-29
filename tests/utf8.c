#include "macros.h"
#include "gfx.h"
#include "screen.h"

int main(void)
{
    Rect rs[3], r;
    int sel = ARRAY_SIZE(rs) - 1;
    int wraps[3] = { 0, 0, 0 };
    const char *texts[] = {
        "Hello there, this is a cool text that is also meant to be\n\n long \nto test the functionality of this thing, the line breaks\nand such.. looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong",
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent vel risus quis quam interdum porttitor. Pellentesque $rSECRET$0 eget lacinia metus, sit amet tristique nunc. Morbi molestie non velit sed porta. Pellentesque placerat quam massa, at egestas lorem accumsan elementum. Integer lectus nulla, aliquam et dui sed, pulvinar bibendum ex. Curabitur eleifend euismod mi, vitae egestas sapien. In accumsan urna at diam placerat, sed tincidunt nisi pharetra. Etiam suscipit tempor augue, suscipit dictum ante auctor at. Nam eleifend elit ac facilisis semper.  Donec vel metus quam. Fusce pellentesque sem euismod mi molestie fermentum. Nunc urna magna, vestibulum non porta eget, tempus nec erat. Nunc in cursus mi, eget elementum velit. Donec ut molestie massa. Praesent accumsan sem mauris, sit amet fermentum purus fermentum pellentesque. Maecenas aliquet nunc ac ante laoreet, at convallis magna dignissim. Suspendisse non libero lectus. Fusce bibendum vitae eros ac porta. Cras eget interdum ligula. Ut et libero vitae lacus mattis feugiat. Morbi ac mollis ligula.",
        "$b27$0無べひ動位ヱカヌ毎受増ソ継7面すろわリ次四給ぜめラえ正知リヘスク父占ぎれーろ禁伊マエルモ受5携うてむふ線9首寄静宙そじ。悪オセヱ気再ひえは者加変イ祖点ニマキ講合ぞへゆあ芸利クカメモ聞方整無療びぶよフ勝能クハ投捜んえもゃ。$g降者$0げぐ海模ヱヌ仕求ちた安同響ぐせども消道ふ三携リ日文げ済禁そ社景ノユモメ井問ソツネセ編49浜け技府ルヌメ業定ネ含断メホサリ量物ンへは府味ラて一円危尊わレぼお。",
    };

    InitScreen();

    for (int i = 0; i < (int) ARRAY_SIZE(rs); i++) {
        rs[i].x = i * COLS / 5;
        rs[i].y = 1 + i * LINES / 5;
        rs[i].w = COLS / 4;
        rs[i].h = LINES / 3;
    }

    while (1) {
        erase();
        attr_set(A_REVERSE, 0, NULL);
        mvaddstr(0, 0, "w toggle wrap, $ - toggle dseq, space switch window, +/- to resize, q to quit");

        for (int i = 0; i < (int) ARRAY_SIZE(rs); i++) {
            r = rs[i];
            attr_set(A_NORMAL, 0, NULL);
            const char *win = i != sel ? "Window" : "$r$dWindow$0";
            DrawBox(win, &r);

            r.x++;
            r.y++;
            r.w -= 2;
            r.h -= 2;
            DrawString(stdscr, &r, wraps[i], texts[i]);
        }

        const int c = getch();
        if (c == 'q') {
            break;
        }
        switch (c) {
        case '$':
            wraps[sel] ^= DS_DSEQ;
            break;
        case 'w':
            wraps[sel] ^= DS_WRAP;
            break;
        case ' ':
            sel++;
            sel %= ARRAY_SIZE(rs);
            break;
        case '+':
            rs[sel].w += 2;
            rs[sel].h += 2;
            break;
        case '-':
            rs[sel].w -= 2;
            rs[sel].h -= 2;
            break;
        }
        if (rs[sel].x + rs[sel].w > COLS) {
            rs[sel].w = COLS - rs[sel].x;
        }
        if (rs[sel].y + rs[sel].h > LINES) {
            rs[sel].h = LINES - rs[sel].y;
        }
        if (rs[sel].w < 3) {
            rs[sel].w = 3;
        }
        if (rs[sel].h < 3) {
            rs[sel].h = 3;
        }
    }

    EndScreen();
    return 0;
}
