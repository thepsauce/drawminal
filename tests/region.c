#include "macros.h"
#include "geom.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*static void PrintRgn(struct rgn *rgn)
{
    printf("%u rect(s)\n", rgn->n);
    for (uint32_t i = 0; i < rgn->n; i++) {
        Rect *const r = &rgn->r[i];
        printf("\t%d, %d, %d, %d\n", r->x, r->y, r->w, r->h);
    }
}*/

static struct rgn *RandRgn(void)
{
    struct rgn *rgn;

    rgn = CreateRgn();
    if (rgn == NULL) {
        return NULL;
    }
    const uint32_t n = (uint64_t) rand() * 100 / RAND_MAX;
    for (uint32_t i = 0; i < n; i++) {
        Rect r;

        r.x = rand();
        r.y = rand();
        r.w = rand();
        r.h = rand();
        if (AddRectRgn(rgn, &r) == NULL) {
            return NULL;
        }
    }
    return rgn;
}

static uint32_t SomeRegions(struct rgn **rgns, uint32_t max)
{
    const uint32_t n = 1 + (uint64_t) rand() * (max - 1) / RAND_MAX;
    for (uint32_t i = 0; i < n; i++) {
        rgns[i] = RandRgn();
        if (rgns[i] == NULL) {
            return 0;
        }
    }
    return n;
}

int main(void)
{
    time_t seed;
    const int tries = 256;
    struct rgn *rgns[3];

    seed = time(NULL);
    srand(seed);

    printf("seed: %ld\n", seed);
    printf("trying %d times a combination of operations\n", tries);

    for (int i = 0; i < tries; i++) {
        struct rgn *rgn1, *rgn2, *rgn3;

        printf("%d) \n", i);
        switch (SomeRegions(rgns, ARRAY_SIZE(rgns))) {
        case 0:
            return -1;
        case 1:
            rgn1 = rgn2 = rgn3 = rgns[0];
            break;
        case 2:
            switch (rand() % 3) {
            case 0:
                rgn1 = rgn2 = rgns[0];
                rgn3 = rgns[1];
                break;
            case 1:
                rgn1 = rgns[0];
                rgn2 = rgn3 = rgns[0];
                break;
            case 2:
                rgn1 = rgns[0];
                rgn2 = rgn3 = rgns[0];
                break;
            }
        }
        for (int i = 0; i < 6; i++) {
            if (rand() % 2 == 0) {
                IntersectRgn(rgn1, rgn2, rgn3);
            }
            if (rand() % 2 == 0) {
                AddRgn(rgn1, rgn2, rgn3);
            }
            rgn1->b = (Rect) { 0, 0, MAX((uint16_t) rand(), 3333), MAX((uint16_t) rand(), 3333) };
            rgn2->b = (Rect) { 0, 0, MAX((uint16_t) rand(), 3333), MAX((uint16_t) rand(), 3333) };
            rgn3->b = (Rect) { 0, 0, MAX((uint16_t) rand(), 3333), MAX((uint16_t) rand(), 3333) };
            if (rand() % 2 == 0) {
                SubtractRgn(rgn1, rgn2, rgn3);
            }
            rgn1->b = (Rect) { 0, 0, MAX((uint16_t) rand(), 3333), MAX((uint16_t) rand(), 3333) };
            rgn2->b = (Rect) { 0, 0, MAX((uint16_t) rand(), 3333), MAX((uint16_t) rand(), 3333) };
            rgn3->b = (Rect) { 0, 0, MAX((uint16_t) rand(), 3333), MAX((uint16_t) rand(), 3333) };
            if (rand() % 2 == 0) {
                InvertRgn(rgn2, rgn3);
            }
        }
    }

    return 0;
}
