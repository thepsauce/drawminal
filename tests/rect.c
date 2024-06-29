#include "macros.h"
#include "geom.h"

#include <stdio.h>

void PrintRect(const Rect *r)
{
    printf("(%d, %d, %d, %d)", r->x, r->y, r->w, r->h);
}

int main(void)
{
    Rect rects[2] = {
        { 30, 50, 80, 20 },
        { 100, 20, 40, 30 }
    };
    Rect r, r2;

    r = (Rect) { 20, 20, 40, 40 };
    for (uint32_t i = 0; i < ARRAY_SIZE(rects); i++) {
        printf("intersection of ");
        PrintRect(&r);
        printf(" and ");
        PrintRect(&rects[i]);
        printf(" is: ");
        if (IntersectRect(&r, &rects[i], &r2)) {
            PrintRect(&r2);
        } else {
            printf("nothing!");
        }
        printf("\n");
    }

    return 0;
}
