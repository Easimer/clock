#include <assert.h>
#include "config.h"
#include "display.h"

static void decomposeDigits(uint8_t num, uint8_t *dh, uint8_t *dl) {
    *dh = num / 10;
    *dl = num % 10;
}

display_status_t displayInit(display_state_t *display) {
    assert(display != NULL);
    if (display == NULL) {
        return EDISP_INVALID_ARG;
    }

    display->currentView = EDISPVIEW_CLOCK;
    for (uint8_t i = 0; i < EDISPVIEW_MAX; i++) {
        display->viewState[i].hour = 0;
        display->viewState[i].minute = 0;
        display->viewState[i].second = 0;
    }

    return EDISP_OK;
}

display_status_t displayUpdateTime(display_state_t *display, display_view_t view, timekeeper_t *tk) {
    if (display == NULL || tk == NULL) {
        return EDISP_INVALID_ARG;
    }

    if (view < 0 || view >= EDISPVIEW_MAX) {
        return EDISP_INVALID_ARG;
    }

    timekeeperGet(tk, &display->viewState[view].hour, &display->viewState[view].minute, &display->viewState[view].second);

    if (display->currentView == view) {
        display->hw->showTime(display->user, display->viewState[view].hour, display->viewState[view].minute, display->viewState[view].second);
    }

    return EDISP_OK;
}

display_status_t displaySwitchView(display_state_t *display, display_view_t view) {
    if (display == NULL || view < 0 || view >= EDISPVIEW_MAX) {
        return EDISP_INVALID_ARG;
    }

    if (display->currentView == view) {
        return EDISP_OK;
    }

    display->currentView = view;
    display->hw->showTime(display->user, display->viewState[view].hour, display->viewState[view].minute, display->viewState[view].second);
    display->hw->showIcon(display->user, EDISPICON_CLOCK + view);

    return EDISP_OK;
}