#include "utest.h"
#include "config.h"
#include "core.h"
#include "ram_eeprom.h"

typedef struct Core {
    core_state_t coreState;

    ram_eeprom_buffer_t memory;
    eeprom_access_t memoryAccess;

    display_hardware_t displayHw;
    display_state_t display;

    uint8_t displayHour, displayMinute, displaySecond;
    display_icon_t displayIcon;

    uint8_t buttonStates[4];
} state_t;

static uint8_t coreButtonProbe(void *user, uint8_t btnIdx);
static display_hardware_status_t showTime(void *user, uint8_t hour, uint8_t minute, uint8_t second);
static display_hardware_status_t showIcon(void *user, display_icon_t icon);

UTEST_F_SETUP(Core) {
    int rc;
    utest_fixture->coreState.buttonProbeUser = utest_fixture;
    utest_fixture->coreState.buttonProbe = coreButtonProbe;
    utest_fixture->coreState.externalMemory = &utest_fixture->memoryAccess;
    ramEepromFillDescriptor(&utest_fixture->memoryAccess, &utest_fixture->memory);
    ramEepromClear(&utest_fixture->memory);

    utest_fixture->displayHw.showIcon = showIcon;
    utest_fixture->displayHw.showTime = showTime;
    utest_fixture->display.hw = &utest_fixture->displayHw;
    utest_fixture->display.user = utest_fixture;
    utest_fixture->coreState.display = &utest_fixture->display;

    rc = coreInit(&utest_fixture->coreState);
    ASSERT_EQ(rc, 0);
    timerEnable(TIMER_ID1);
}

UTEST_F_TEARDOWN(Core) {
}

UTEST_F(Core, InitialClockTime) {
    ASSERT_EQ(utest_fixture->displayHour, 0);
    ASSERT_EQ(utest_fixture->displayMinute, 0);
    ASSERT_EQ(utest_fixture->displaySecond, 0);
    ASSERT_EQ(utest_fixture->displayIcon, EDISPICON_CLOCK);
}

UTEST_F(Core, SetMinuteButton) {
    int rc;
    uint8_t hour, minute, second;
    timekeeper_t *tkClock;

    tkClock = coreGetClock(&utest_fixture->coreState);

    // Press the `set time` button five times
    for (int i = 0; i < 5; i++) {
        utest_fixture->buttonStates[3] = 1;
        rc = coreElapsed(&utest_fixture->coreState, 25);
        utest_fixture->buttonStates[3] = 0;
        rc = coreElapsed(&utest_fixture->coreState, 1);

        for (int i = 0; i < ACTIONS_DOUBLE_CLICK_MAX_MILLISECONDS_SINCE_LAST_RELEASE; i++) {
            rc = coreElapsed(&utest_fixture->coreState, 1);
        }
    }

    timekeeperGet(tkClock, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 5);
    ASSERT_EQ(second, 0);
}

static uint8_t coreButtonProbe(void *user, uint8_t btnIdx) {
    state_t *state = (state_t *)user;
    if (btnIdx >= 4) {
        return 0;
    }
    
    return state->buttonStates[btnIdx];
}

static display_hardware_status_t showTime(void *user, uint8_t hour, uint8_t minute, uint8_t second) {
    state_t *state = (state_t *)user;

    state->displayHour = hour;
    state->displayMinute = minute;
    state->displaySecond = second;

    return EDISPHW_OK;
}

static display_hardware_status_t showIcon(void *user, display_icon_t icon) {
    state_t *state = (state_t *)user;

    state->displayIcon = icon;

    return EDISPHW_OK;
}

void logPrintString(char const *s) {
    printf("%s", s);
}

void logPrintNumber(uint16_t n) {
    printf("%u", n);
}