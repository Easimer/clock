#include "config.h"
#include "display.h"
#include "utest.h"

static display_hardware_status_t cbShowTime(void *user, uint8_t hour, uint8_t minute, uint8_t second);
static display_hardware_status_t cbShowIcon(void *user, display_icon_t icon);

typedef struct Display {
    display_icon_t icon;
    uint8_t hour, minute, second;

    display_hardware_t hw;
    display_state_t disp;
} display_test_t;

UTEST_F_SETUP(Display) {
    display_status_t rc;

    utest_fixture->hw.showTime = &cbShowTime;
    utest_fixture->hw.showIcon = &cbShowIcon;

    utest_fixture->disp.hw = &utest_fixture->hw;
    utest_fixture->disp.user = utest_fixture;

    rc = displayInit(&utest_fixture->disp);
    ASSERT_EQ(rc, EDISP_OK);
}

UTEST_F_TEARDOWN(Display) {
}

UTEST_F(Display, InitialState) {
    display_status_t rc;
    rc = displayInit(&utest_fixture->disp);
    ASSERT_EQ(rc, EDISP_OK);

    ASSERT_EQ(utest_fixture->disp.currentView, EDISPVIEW_CLOCK);
}

UTEST_F(Display, ViewSwitch) {
    display_status_t rc;
    rc = displaySwitchView(&utest_fixture->disp, EDISPVIEW_STOPWATCH);
    ASSERT_EQ(rc, EDISP_OK);
    ASSERT_EQ(utest_fixture->disp.currentView, EDISPVIEW_STOPWATCH);
    ASSERT_EQ(utest_fixture->icon, EDISPICON_STOPWATCH);
}

UTEST_F(Display, SetTimeOnCurrentView) {
    TIMEKEEPER_DECLARE_BUFFER(tkTime);
    timekeeperSet(tkTime, 14, 23, 45);

    display_status_t rc;
    rc = displaySwitchView(&utest_fixture->disp, EDISPVIEW_STOPWATCH);
    ASSERT_EQ(rc, EDISP_OK);
    rc = displayUpdateTime(&utest_fixture->disp, EDISPVIEW_STOPWATCH, tkTime);
    ASSERT_EQ(rc, EDISP_OK);

    ASSERT_EQ(utest_fixture->hour, 14);
    ASSERT_EQ(utest_fixture->minute, 23);
    ASSERT_EQ(utest_fixture->second, 45);
}

UTEST_F(Display, SetTimeOnOtherView) {
    TIMEKEEPER_DECLARE_BUFFER(tkTime);
    timekeeperSet(tkTime, 14, 23, 45);

    utest_fixture->hour = 0;
    utest_fixture->minute = 0;
    utest_fixture->second = 0;

    display_status_t rc;
    rc = displaySwitchView(&utest_fixture->disp, EDISPVIEW_STOPWATCH);
    ASSERT_EQ(rc, EDISP_OK);
    rc = displayUpdateTime(&utest_fixture->disp, EDISPVIEW_CLOCK, tkTime);
    ASSERT_EQ(rc, EDISP_OK);

    ASSERT_EQ(utest_fixture->hour, 0);
    ASSERT_EQ(utest_fixture->minute, 0);
    ASSERT_EQ(utest_fixture->second, 0);
}

static display_hardware_status_t cbShowTime(void *user, uint8_t hour, uint8_t minute, uint8_t second) {
    display_test_t *D = (display_test_t *)user;

    D->hour = hour;
    D->minute = minute;
    D->second = second;

    return EDISPHW_OK;
}

static display_hardware_status_t cbShowIcon(void *user, display_icon_t icon) {
    display_test_t *D = (display_test_t *)user;

    D->icon = icon;

    return EDISPHW_OK;
}
