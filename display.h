#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

#include <stddef.h>
#include <stdint.h>
#include "timekeeper.h"

typedef enum display_hardware_status {
    EDISPHW_OK = 0,
    EDISPHW_UNSUPPORTED,
} display_hardware_status_t;

typedef enum display_icon {
    EDISPICON_BASE = 0,
    EDISPICON_CLOCK = 0,
    EDISPICON_STOPWATCH,
    EDISPICON_MAX
} display_icon_t;

typedef display_hardware_status_t (*display_hardware_show_time)(void *user, uint8_t hour, uint8_t minute, uint8_t second);
typedef display_hardware_status_t (*display_hardware_show_icon)(void *user, display_icon_t icon);

typedef struct display_hardware {
    display_hardware_show_time showTime;
    display_hardware_show_icon showIcon;
} display_hardware_t;

typedef enum display_view {
    EDISPVIEW_BASE = 0,
    EDISPVIEW_CLOCK = 0,
    EDISPVIEW_STOPWATCH,
    EDISPVIEW_MAX
} display_view_t;

typedef struct display_state {
    display_hardware_t *hw;
    void *user;
    display_view_t currentView;

    struct {
        uint8_t hour, minute, second;
    } viewState[EDISPVIEW_MAX];
} display_state_t;

typedef enum display_status {
    EDISP_OK = 0,
    EDISP_INVALID_ARG,
} display_status_t;

#ifdef __cplusplus
extern "C" {
#endif

display_status_t displayInit(display_state_t *display);
display_status_t displayUpdateTime(display_state_t *display, display_view_t view, timekeeper_t *tk);
display_status_t displaySwitchView(display_state_t *display, display_view_t view);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_DISPLAY_H */
