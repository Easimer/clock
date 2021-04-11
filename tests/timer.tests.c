#include "utest.h"
#include "config.h"
#include "timer.h"

typedef struct Timer {
    timer_subscription_t subscription;
    int callbackCalled;
    uint16_t millisElapsed;
} state_t;

static void timerCallback(void *user, uint16_t millisElapsed) {
    state_t *state = (state_t *)user;

    state->callbackCalled = 1;
    state->millisElapsed = millisElapsed;
}

UTEST_F_SETUP(Timer) {
    timerSetup(TIMER_ID1);

    utest_fixture->callbackCalled = 0;
    utest_fixture->millisElapsed = 0;

    int rc = timerSubscribe(TIMER_ID1, &utest_fixture->subscription, utest_fixture, timerCallback);
    ASSERT_EQ(rc, ETIMER_OK);
}

UTEST_F_TEARDOWN(Timer) {
}

UTEST_F(Timer, NotEnabled) {
    timerAddTimeElapsed(TIMER_ID1, 1);

    ASSERT_EQ(utest_fixture->millisElapsed, 0);
    ASSERT_EQ(utest_fixture->callbackCalled, 0);
}

UTEST_F(Timer, Enabled1Ms) {
    timerEnable(TIMER_ID1);
    timerAddTimeElapsed(TIMER_ID1, 1);

    ASSERT_EQ(utest_fixture->millisElapsed, 1);
    ASSERT_EQ(utest_fixture->callbackCalled, 1);
}

UTEST_F(Timer, Enabled13Ms) {
    timerEnable(TIMER_ID1);
    timerAddTimeElapsed(TIMER_ID1, 13);

    ASSERT_EQ(utest_fixture->millisElapsed, 13);
    ASSERT_EQ(utest_fixture->callbackCalled, 1);
}

UTEST_F(Timer, HandleNULL) {
    int rc;

    rc = timerSubscribe(TIMER_ID1, NULL, NULL, timerCallback);
    ASSERT_EQ(rc, ETIMER_INVAL);
}

UTEST_F(Timer, CallbackNULL) {
    int rc;

    rc = timerSubscribe(TIMER_ID1, &utest_fixture->subscription, NULL, NULL);
    ASSERT_EQ(rc, ETIMER_INVAL);
}

UTEST_F(Timer, MaxSubs) {
    int rc = ETIMER_OK;
    int times = 0;

    while (rc != ETIMER_MAX_SUBSCRIBERS) {
        times++;
        rc = timerSubscribe(TIMER_ID1, &utest_fixture->subscription, NULL, timerCallback);
    }

    ASSERT_EQ(rc, ETIMER_MAX_SUBSCRIBERS);
    ASSERT_EQ(times, TIMER_MAX_SUBSCRIBERS);
}
