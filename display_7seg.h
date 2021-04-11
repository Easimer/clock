#ifndef CLOCK_DISPLAY_7SEG_H
#define CLOCK_DISPLAY_7SEG_H

/*
 * Display abstraction layer for 4 digit 7-segment displays
 */

#include "config.h"
#include "display.h"
#include "7seg.h"

typedef struct display_7seg {
    display_state_t state;
    d7seg_ctl_t *hwCtl;
    display_hardware_t display;
} display_7seg_t;

#ifdef __cplusplus
extern "C" {
#endif

void d7segInitDescriptor(display_7seg_t *d, d7seg_ctl_t *hwCtl);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_DISPLAY_7SEG_H */