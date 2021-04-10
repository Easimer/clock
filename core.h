#ifndef CLOCK_CORE_H
#define CLOCK_CORE_H

#include <stdint.h>

#include "actions.h"
#include "7seg.h"
#include "display.h"
#include "timer.h"
#include "timekeeper.h"
#include "timesave_io.h"

typedef uint8_t (*core_button_probe_t)(uint8_t btnIdx);

typedef struct core_state {
    // ------------------------------------
    // Set by hardware layer
    // ------------------------------------
    d7seg_ctl_t *displayCtl;
    core_button_probe_t buttonProbe;
    eeprom_access_t *externalMemory;

    // ------------------------------------
    // Private
    // ------------------------------------
    actions_button_handle_t btnStopwatch;
    actions_button_handle_t btnSetTime;
    TIMEKEEPER_DECLARE_BUFFER_IN(tkTime);
    uint8_t saveTimeToROM;
    timesave_io_config_t tsiocTime;

    timer_subscription_t subscriptionDisplayDigits;
    timer_subscription_t subscriptionAccumulateTime;

    uint8_t minutesElapsedSinceLastTimeSave;

    display_state_t display;
    display_hardware_t displayHw;
} core_state_t;

#ifdef __cplusplus
extern "C" {
#endif

int coreInit(core_state_t *state);
int coreLoop(core_state_t *state);
int coreElapsed(core_state_t *state, uint8_t millisElapsed);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_CORE_H */