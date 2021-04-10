#include <string.h>

#include "core.h"
#include "log.h"

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
        btnState = state->buttonProbe(2);
    } else if (handle == state->btnSetTime) {
        btnState = state->buttonProbe(3);
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
            l_str_ln("[+] Saved time");
            break;
        case ETIMESAVE_IO_ERASED:
            l_str_ln("[+] timesave: first initialization");
            break;
        case ETIMESAVE_IO_WRITE_FAILURE:
            l_str_ln("[-] timesave: couldn't save time: write failure");
            break;
        default:
            break;
    }
}

static void restoreTimeFromEEPROM(core_state_t *state, timekeeper_t *tk) {
    timesave_io_status_t rc;

    l_str_ln("[+] Restoring time from EEPROM...");

    rc = restoreTime(tk, &state->tsiocTime);

    switch (rc) {
        case ETIMESAVE_IO_OK:
            l_str_ln("[+] Restored time");
            break;
        case ETIMESAVE_IO_BADSIG:
            l_str_ln("[-] Couldn't restore time: bad signature (old version?)");
            break;
        case ETIMESAVE_IO_BADCHK:
            l_str_ln("[-] Couldn't restore time: bad checksum");
            break;
        case ETIMESAVE_IO_READ_FAILURE:
            l_str_ln("[-] Couldn't restore time: read failure");
            break;
        case ETIMESAVE_IO_WRITE_FAILURE:
            l_str_ln("[-] Couldn't restore time: write failure");
            break;
        case ETIMESAVE_IO_ERASED:
            l_str_ln("[+] Time wasn't restored: first initialization");
            break;
        default:
            break;
    }
}

static void displayDigits(void *user, uint16_t millis_elapsed) {
    core_state_t *state = (core_state_t *)user;
    displayDigitsDec(state->displayCtl, state->digits[0], state->digits[1], state->digits[2], state->digits[3]);
}

static void accumulateTime(void *user, uint16_t millisElapsed) {
    core_state_t *state = (core_state_t *)user;
    int change;
    if ((change = timekeeperAccumulate(state->tkTime, millisElapsed)) != 0) {
        uint8_t hours, minutes, seconds;
        timekeeperGet(state->tkTime, &hours, &minutes, &seconds);

        decomposeDigits(hours, &state->digits[0], &state->digits[1]);
        decomposeDigits(minutes, &state->digits[2], &state->digits[3]);

        if (change > 1 || change < -1) {
            state->minutesElapsedSinceLastTimeSave += 1;

            if (state->minutesElapsedSinceLastTimeSave >= 5) {
                state->saveTimeToROM = 1;
                state->minutesElapsedSinceLastTimeSave = 0;
            }
        }
    }
}

static void decomposeDigits(uint8_t num, uint8_t *dh, uint8_t *dl) {
    *dh = num / 10;
    *dl = num % 10;
}

int coreInit(core_state_t *state) {
    int rc;

    TIMEKEEPER_SETUP_BUFFER_IN(state, tkTime);
    state->saveTimeToROM = 0;
    state->minutesElapsedSinceLastTimeSave = 0;
    memset(state->digits, 0, sizeof(state->digits));

    timerSetup(TIMER_ID1);

    l_str_ln("[+] Creating main timekeeper");
    timekeeperInit(state->tkTime);

    l_str_ln("[+] Initializing time save/restore mechanism");
    timesave_io_status_t tsioStatus = fillTimesaveConfig(&state->tsiocTime, state->externalMemory, 0);
    if (tsioStatus != ETIMESAVE_IO_OK) {
        l_str("[+] timesave: fillTimesaveConfig failed with rc=");
        l_num_ln(tsioStatus);
    }
    else {
        restoreTimeFromEEPROM(state, state->tkTime);
    }

    l_str_ln("[+] Setting up actions subsystem");
    actionsInit();

    if ((rc = actionsCreateButton(&state->btnSetTime, state, &btnCommonDescriptor)) != EACTIONS_OK) {
        l_str("[!] actionsCreateButton failed: ");
        l_num_ln(rc);
    }

    if ((rc = actionsCreateButton(&state->btnStopwatch, state, &btnCommonDescriptor)) != EACTIONS_OK) {
        l_str("[!] actionsCreateButton failed: ");
        l_num_ln(rc);
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
