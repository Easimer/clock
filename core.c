#include <string.h>

#include "core.h"
#include "config.h"
#include "kprintf.h"

static void probeButton(actions_button_handle_t handle, void *user);
static void clickButton(actions_button_handle_t handle, void *user);
static void longPress(actions_button_handle_t handle, void *user);

static void saveTimeToEEPROM(core_state_t *state, timekeeper_t const *tk);
static void restoreTimeFromEEPROM(core_state_t *state, timekeeper_t *tk);

static void displayDigits(void *user, uint16_t millis_elapsed);
static void accumulateTime(void *user, uint16_t millisElapsed);
static void decomposeDigits(uint8_t num, uint8_t *dh, uint8_t *dl);

static actions_button_descriptor_t btnCommonDescriptor = {
    .probe = probeButton,
    .pressed = NULL,
    .released = NULL,
    .click = clickButton,
    .longPress = longPress,
    .doubleClick = NULL,
};

static void probeButton(actions_button_handle_t handle, void *user) {
    core_state_t *state = (core_state_t *)user;
    uint8_t btnState = 0;

    if (handle == state->btnStopwatch) {
        btnState = state->buttonProbe(state->buttonProbeUser, 2);
    } else if (handle == state->btnSetTime) {
        btnState = state->buttonProbe(state->buttonProbeUser, 3);
    }

    actionsSetButtonState(handle, btnState);
}

static void clickButton(actions_button_handle_t handle, void *user) {
    core_state_t *state = (core_state_t *)user;

    if (handle == state->btnSetTime) {
        uint8_t hours, minutes, seconds;
        timekeeperGet(state->tkTime, &hours, &minutes, &seconds);
        minutes++; // timekeeperSet will handle any overflow
        timekeeperSet(state->tkTime, hours, minutes, seconds);
    }
}

static void longPress(actions_button_handle_t handle, void *user) {
    core_state_t *state = (core_state_t *)user;

    if (handle == state->btnSetTime) {
        uint8_t hours, minutes, seconds;
        timekeeperGet(state->tkTime, &hours, &minutes, &seconds);
        hours++; // timekeeperSet will handle any overflow
        timekeeperSet(state->tkTime, hours, minutes, seconds);
    }
}

static void saveTimeToEEPROM(core_state_t *state, timekeeper_t const *tk) {
    timesave_io_status_t rc;
    rc = saveTime(tk, &state->tsiocTime);

    switch (rc) {
        case ETIMESAVE_IO_OK:
            kprintf(LOG_SUCCESS "timesave: saved time\n");
            break;
        case ETIMESAVE_IO_ERASED:
            kprintf(LOG_SUCCESS "timesave: first initialization\n");
            break;
        case ETIMESAVE_IO_WRITE_FAILURE:
            kprintf(LOG_FAILURE "timesave: couldn't save time: write failure\n");
            break;
        default:
            break;
    }
}

static void restoreTimeFromEEPROM(core_state_t *state, timekeeper_t *tk) {
    timesave_io_status_t rc;

    kprintf(LOG_SUCCESS "Restoring time from EEPROM...\n");

    rc = restoreTime(tk, &state->tsiocTime);

    switch (rc) {
        case ETIMESAVE_IO_OK:
            kprintf(LOG_SUCCESS "Restored time\n");
            break;
        case ETIMESAVE_IO_BADSIG:
            kprintf(LOG_FAILURE "Couldn't restore time: bad signature (old version?)\n");
            break;
        case ETIMESAVE_IO_BADCHK:
            kprintf(LOG_FAILURE "Couldn't restore time: bad checksum\n");
            break;
        case ETIMESAVE_IO_READ_FAILURE:
            kprintf(LOG_FAILURE "Couldn't restore time: read failure\n");
            break;
        case ETIMESAVE_IO_WRITE_FAILURE:
            kprintf(LOG_FAILURE "Couldn't restore time: write failure\n");
            break;
        case ETIMESAVE_IO_ERASED:
            kprintf(LOG_SUCCESS "Time wasn't restored: first initialization\n");
            break;
        default:
            break;
    }
}

static void displayDigits(void *user, uint16_t millis_elapsed) {
    core_state_t *state = (core_state_t *)user;
    displayUpdateTime(state->display, EDISPVIEW_CLOCK, state->tkTime);
}

static void accumulateTime(void *user, uint16_t millisElapsed) {
    core_state_t *state = (core_state_t *)user;
    int change;
    if ((change = timekeeperAccumulate(state->tkTime, millisElapsed)) != 0) {
        uint8_t hours, minutes, seconds;
        timekeeperGet(state->tkTime, &hours, &minutes, &seconds);

        if (change > 1 || change < -1) {
            state->minutesElapsedSinceLastTimeSave += 1;

            if (state->minutesElapsedSinceLastTimeSave >= TIMESAVE_INTERVAL_MINUTES) {
                state->saveTimeToROM = 1;
                state->minutesElapsedSinceLastTimeSave = 0;
            }
        }
    }
}

int coreInit(core_state_t *state) {
    uint8_t rc;

    TIMEKEEPER_SETUP_BUFFER_IN(state, tkTime);
    state->saveTimeToROM = 0;
    state->minutesElapsedSinceLastTimeSave = 0;

    timerSetup(TIMER_ID1);

    kprintf(LOG_SUCCESS "Creating main timekeeper\n");
    timekeeperInit(state->tkTime);

    kprintf(LOG_SUCCESS "Initializing time save/restore mechanism\n");
    timesave_io_status_t tsioStatus = fillTimesaveConfig(&state->tsiocTime, state->externalMemory, 0);
    if (tsioStatus != ETIMESAVE_IO_OK) {
        kprintf(LOG_FAILURE "timesave: fillTimesaveConfig failed with rc=%b\n", (uint8_t)tsioStatus);
    }
    else {
        restoreTimeFromEEPROM(state, state->tkTime);
    }

    kprintf(LOG_SUCCESS "Setting up actions subsystem\n");
    actionsInit();

    if ((rc = actionsCreateButton(&state->btnSetTime, state, &btnCommonDescriptor)) != EACTIONS_OK) {
        kprintf(LOG_ERROR "actionsCreateButton failed: %b\n", rc);
    }

    if ((rc = actionsCreateButton(&state->btnStopwatch, state, &btnCommonDescriptor)) != EACTIONS_OK) {
        kprintf(LOG_ERROR "actionsCreateButton failed: %b\n", rc);
    }

    kprintf(LOG_SUCCESS "Initializing display subsystem\n");
    if ((rc = displayInit(state->display)) != EDISP_OK) {
        kprintf(LOG_ERROR "displayInit failed: %b\n", rc);
    }

    timerSubscribe(TIMER_ID1, &state->subscriptionDisplayDigits, state, displayDigits);
    timerSubscribe(TIMER_ID1, &state->subscriptionAccumulateTime, state, accumulateTime);

    return 0;
}

int coreLoop(core_state_t *state) {
    if (state->saveTimeToROM) {
        state->saveTimeToROM = 0;
        saveTimeToEEPROM(state, state->tkTime);
    }

    return 0;
}

int coreElapsed(core_state_t *state, uint8_t millisElapsed) {
    timerAddTimeElapsed(TIMER_ID1, 1);
    return 0;
}


timekeeper_t *coreGetClock(core_state_t *state) {
    return state->tkTime;
}
