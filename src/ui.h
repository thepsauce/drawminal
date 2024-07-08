#ifndef UI_H
#define UI_H

#include "geom.h"
#include "gfx.h"
#include "screen.h"

#include <stdbool.h>
#include <time.h>

enum hist_event_type {
    /* selected region was changed */
    HEV_SELCHG,
    /* selected region was deleted */
    HEV_SELDEL,
    /* selected region was placed */
    HEV_SELPLC,
    /* a brush stroke was performed */
    HEV_STROKE,
};

/*
 * history node, the history is a tree, not linear
 */
struct hist_event {
    enum hist_event_type type;
    time_t time;
    /* the region selected on this event */
    struct rgn *rgn;
    union {
        struct stroke st;
        struct {
            /* previous data */
            WINDOW *win;
            /* place data */
            WINDOW *pwin;
            int dx, dy;
        };
    };
    /* parent event */
    struct hist_event *p;
    /* child events */
    struct hist_event **c;
    /* number of child events */
    uint32_t n;
};

/* note: the history must be initialized by setting cur to &root */
struct history {
    struct hist_event root;
    struct hist_event *cur;
};

/*
 * calloc()-es and returns a struct with type set to the given type and
 * time set to the current time, it also adds this event to the history,
 * use DropEvent to delete the event and remove it again
 */
struct hist_event *CreateEvent(struct history *hist,
        enum hist_event_type type, const struct rgn *rgn);

void DropEvent(struct history *hist);

/*
 * clears all history downwards from cur node
 */
void ClearBranch(struct history *hist);

struct canvas {
    /* coordinates on the screen */
    int x, y, w, h;
    /* drawing data */
    WINDOW *data;
    /* all user actions */
    struct history hist;
    /* selected area */
    struct rgn *sel;
    /* cached areas under the selection */
    WINDOW **cache;
};

/*
 * caches the selected region sel, storing it in cache
 */
int UpdateCache(struct canvas *cv);

/*
 * these return true when there was an event to redo/undo and false
 * if nothing is to be done
 */
bool UndoEvent(struct canvas *cv);
bool RedoEvent(struct canvas *cv);

extern struct canvases {
    struct canvas *p;
    uint32_t n;
    /* selected canvas */
    uint32_t sel;
} Canvases;

struct tool {
    /* tool identifier */
    const char *name;
    /* event handler for this tool */
    int (*handle)(struct tool *tool, struct canvas *cv, struct event *ev);
    /* tool data */
    union {
        /* brush/eraser */
        struct {
            /* current brush stroke */
            struct stroke st;
        };
        /* selection */
        struct {
            /* true if a selection is being made */
            bool sel;
            /* true if a selection is being moved */
            bool move;
            /* accumulated delta position of this movement */
            int dx, dy;
            /* coordinates are relative to the canvas origin */
            int sx, sy, ex, ey;
            /* append mode */
            bool am;
            /* subtract mode */
            bool sm;
        };
    };
};

extern struct toolbar {
    /* coordinates on the screen */
    int x, y, w, h;
    /* tools this toolbar has */
    struct tool *t;
    /* number of those tools */
    unsigned n;
    /* selected tool */
    unsigned sel;
} Toolbar;

extern struct colorpicker {
    /* coordinates on the screen */
    int x, y, w, h;
} ColorPicker;

int saveCanvas(struct canvas *cv, struct event *ev, const char *file_path);

int loadCanvas(struct canvas *cv, struct event *ev, const char *file);

#endif
