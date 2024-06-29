#ifndef GEOM_H
#define GEOM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct point {
    int32_t x, y;
} Point;

typedef struct rect {
    int32_t x, y, w, h;
} Rect;

/*
 * checks if the width or height is 0
 */
bool IsRectEmpty(const Rect *rect);

bool RectContains(const Rect *r, const Point *p);

/*
 * sets rect to one that perfectly surrounds both r1 and r2
 */
void RectUnion(const Rect *r1, const Rect *r2, Rect *rect);

/*
 * cuts r1 into pieces by removing r2 from it,
 * rects must have enough space to hold 4 rectangles
 */
int CutRect(const Rect *r1, const Rect *r2, Rect *rects);

/*
 * returns if two rectangles intersect and store the intersection
 * in rect (leaves rect untouched if they do not intersect)
 *
 * rect can be null
 */
bool IntersectRect(const Rect *r1, const Rect *r2, Rect *rect);

struct rgn {
    /* rectangles inside this region */
    Rect *r;
    /* number of those rectangles */
    uint32_t n;
    /* bounds surrounding all rectangles */
    Rect b;
};

/*
 * allocates a new region and copies the given rgn or
 * initializes everything to 0 when rgn is NULL
 */
struct rgn *CreateRgn(const struct rgn *rgn);
struct rgn *SetRgnEmpty(struct rgn *rgn);
struct rgn *InvertRgn(struct rgn *rgn, const struct rgn *rgn1);
struct rgn *CreateRectRgn(const Rect *rect);
struct rgn *SetRectRgn(struct rgn *rgn, const Rect *rect);
struct rgn *AddRectRgn(struct rgn *rgn, const Rect *rect);
struct rgn *SubtractRectRgn(struct rgn *rgn, const Rect *rect);
struct rgn *IntersectRgn(struct rgn *rgn, const struct rgn *rgn1, const struct rgn *rgn2);
struct rgn *AddRgn(struct rgn *rgn, const struct rgn *rgn1, const struct rgn *rgn2);
struct rgn *SubtractRgn(struct rgn *rgn, const struct rgn *rgn1, const struct rgn *rgn2);
struct rgn *MoveRgnBy(struct rgn *rgn, int32_t dx, int32_t dy);
void DeleteRgn(struct rgn *rgn);

#endif
