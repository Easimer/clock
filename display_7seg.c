#include "display.h"
#include "display_7seg.h"

#if DISPLAY_USE_7SEG

static void decomposeDigits(uint8_t num, uint8_t *dh, uint8_t *dl) {
    *dh = num / 10;
    *dl = num % 10;
}

static display_hardware_status_t displayShowTime(void *user, uint8_t hour, uint8_t minute, uint8_t second) {
    display_7seg_t *state = (display_7seg_t*)user;
    display_view_t currentView = EDISPVIEW_CLOCK;
    uint8_t d0, d1, d2, d3;

    displayGetCurrentView(&state->display, &currentView);

    if (currentView == EDISPVIEW_CLOCK) {
        decomposeDigits(hour, &d0, &d1);
        decomposeDigits(minute, &d2, &d3);
    } else {
        if (hour == 0) {
            decomposeDigits(minute, &d0, &d1);
            decomposeDigits(second, &d2, &d3);
        } else {
            decomposeDigits(hour, &d0, &d1);
            decomposeDigits(minute, &d2, &d3);
        }
    }
    
    d7segDisplayDec(state->hwCtl, d0, d1, d2, d3);

    return EDISPHW_OK;
}

static display_hardware_status_t displayShowIcon(void *user, display_icon_t icon) {
    return EDISPHW_UNSUPPORTED;
}

void d7segInitDescriptor(display_7seg_t *d, d7seg_ctl_t *hwCtl) {
    d->hwCtl = hwCtl;
    d->display.showTime = displayShowTime;
    d->display.showIcon = displayShowIcon;
}

#endif /* DISPLAY_USE_7SEG */
