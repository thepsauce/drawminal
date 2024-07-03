#include "gfx.h"
#include "ui.h"
#include "geom.h"

#include <stdio.h>
#include <stdlib.h>

inline int SaveStroke(struct stroke *st, FILE *f) {
    if (fwrite(st, sizeof(struct stroke), 1, f) != 1) {
        return -1;
    }
    if (fwrite(st->p, sizeof(Point), st->n, f) != st->n) {
        return -1;
    }
    return 0;
}

inline int SaveHistEvent(struct hist_event *event, FILE *f) {
    if (fwrite(event, sizeof(struct hist_event), 1, f) != 1) {
        return -1;
    }
    if (event->type == HEV_STROKE) {
        if (SaveStroke(&event->st, f) != 0) {
            return -1;
        }
    }
    for (uint32_t i = 0; i < event->n; i++) {
        if (SaveHistEvent(event->c[i], f) != 0) {
            return -1;
        }
    }
    return 0;
}

inline int SaveHistory(struct history *hist, FILE *f) {
    return SaveHistEvent(&hist->root, f);
}

int SaveCanvas(struct canvas *cv, const char *file) {
    FILE *f = fopen(file, "wb");
    if (f == NULL) {
        fprintf(stderr, "Failed to open file for writing: %s\n", file);
        return -1;
    }

    if (fwrite(cv, sizeof(struct canvas), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    if (SaveHistory(&cv->hist, f) != 0) {
        fclose(f);
        return -1;
    }

    if (cv->sel != NULL) {
        if (fwrite(cv->sel, sizeof(struct rgn), 1, f) != 1) {
            fclose(f);
            return -1;
        }
        if (fwrite(cv->sel->r, sizeof(Rect), cv->sel->n, f) != cv->sel->n) {
            fclose(f);
            return -1;
        }
    }

    fclose(f);
    return 0;
}

inline int LoadStroke(struct stroke *st, FILE *f) {
    if (fread(st, sizeof(struct stroke), 1, f) != 1) {
        return -1;
    }
    st->p = malloc(st->n * sizeof(Point));
    if (st->p == NULL) {
        return -1;
    }
    if (fread(st->p, sizeof(Point), st->n, f) != st->n) {
        free(st->p);
        return -1;
    }
    return 0;
}

inline int LoadHistEvent(struct hist_event *event, FILE *f) {
    if (fread(event, sizeof(struct hist_event), 1, f) != 1) {
        return -1;
    }
    if (event->type == HEV_STROKE) {
        if (LoadStroke(&event->st, f) != 0) {
            return -1;
        }
    }
    if (event->n > 0) {
        event->c = malloc(event->n * sizeof(struct hist_event *));
        if (event->c == NULL) {
            return -1;
        }
        for (uint32_t i = 0; i < event->n; i++) {
            event->c[i] = malloc(sizeof(struct hist_event));
            if (event->c[i] == NULL) {
                for (uint32_t j = 0; j < i; j++) {
                    free(event->c[j]);
                }
                free(event->c);
                return -1;
            }
            if (LoadHistEvent(event->c[i], f) != 0) {
                for (uint32_t j = 0; j <= i; j++) {
                    free(event->c[j]);
                }
                free(event->c);
                return -1;
            }
            event->c[i]->p = event;
        }
    }
    return 0;
}

int LoadHistory(struct history *hist, FILE *f) {
    return LoadHistEvent(&hist->root, f);
}

int LoadCanvas(struct canvas *cv, const char *file) {
    FILE *f = fopen(file, "rb");
    if (f == NULL) {
        fprintf(stderr, "Failed to open file for reading: %s\n", file);
        return -1;
    }

    if (fread(cv, sizeof(struct canvas), 1, f) != 1) {
        fclose(f);
        return -1;
    }

    if (LoadHistory(&cv->hist, f) != 0) {
        fclose(f);
        return -1;
    }

    if (cv->sel == NULL) { 
        fclose(f);
        return -1;
    }

    cv->sel = malloc(sizeof(struct rgn));
    if (cv->sel == NULL) {
        fclose(f);
        return -1;
    }
    if (fread(cv->sel, sizeof(struct rgn), 1, f) != 1) {
        free(cv->sel);
        cv->sel = NULL;
        fclose(f);
        return -1;
    }
    cv->sel->r = malloc(cv->sel->n * sizeof(Rect));
    if (cv->sel->r == NULL) {
        free(cv->sel);
        cv->sel = NULL;
        fclose(f);
        return -1;
    }
    if (fread(cv->sel->r, sizeof(Rect), cv->sel->n, f) != cv->sel->n) {
        free(cv->sel->r);
        free(cv->sel);
        cv->sel = NULL;
        fclose(f);
        return -1;
    }


    fclose(f);
    return 0;
}
