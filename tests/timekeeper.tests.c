#include "utest.h"

#include "timekeeper.h"

typedef struct Timekeeper {
    TIMEKEEPER_DECLARE_BUFFER_IN(tk);
} state_t;

UTEST_F_SETUP(Timekeeper) {
    TIMEKEEPER_SETUP_BUFFER_IN(utest_fixture, tk);
    timekeeperInit(utest_fixture->tk);
}

UTEST_F_TEARDOWN(Timekeeper) {
}

UTEST_F(Timekeeper, GetterSetter) {
    for (uint8_t hour = 0; hour < 24; hour++) {
        for (uint8_t minute = 0; minute < 60; minute++) {
            for (uint8_t second = 0; second < 60; second++) {
                timekeeperSet(utest_fixture->tk, hour, minute, second);

                uint8_t hour2, minute2, second2;
                timekeeperGet(utest_fixture->tk, &hour2, &minute2, &second2);
                ASSERT_EQ(hour, hour2);
                ASSERT_EQ(minute, minute2);
                ASSERT_EQ(second, second2);
            }
        }
    }
}

UTEST_F(Timekeeper, SecondOverflow60) {
    uint8_t hour, minute, second;
    timekeeperSet(utest_fixture->tk, 0, 0, 60);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 1);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, SecondOverflow65) {
    uint8_t hour, minute, second;
    timekeeperSet(utest_fixture->tk, 0, 0, 65);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 1);
    ASSERT_EQ(second, 5);
}

UTEST_F(Timekeeper, SecondOverflow120) {
    uint8_t hour, minute, second;
    timekeeperSet(utest_fixture->tk, 0, 0, 120);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 2);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, MinuteOverflow60) {
    uint8_t hour, minute, second;
    timekeeperSet(utest_fixture->tk, 0, 60, 0);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 1);
    ASSERT_EQ(minute, 0);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, MinuteOverflow65) {
    uint8_t hour, minute, second;
    timekeeperSet(utest_fixture->tk, 0, 65, 0);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 1);
    ASSERT_EQ(minute, 5);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, MinuteOverflow120) {
    uint8_t hour, minute, second;
    timekeeperSet(utest_fixture->tk, 0, 120, 0);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 2);
    ASSERT_EQ(minute, 0);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, HourOverflow) {
    uint8_t hour, minute, second;
    timekeeperSet(utest_fixture->tk, 23, 60, 0);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 0);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, Accumulate100ms) {
    uint8_t hour, minute, second;
    int rc;

    rc = timekeeperAccumulate(utest_fixture->tk, 100);
    ASSERT_EQ(rc, 0);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 0);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, Accumulate1000ms) {
    uint8_t hour, minute, second;
    int rc;

    rc = timekeeperAccumulate(utest_fixture->tk, 1000);
    ASSERT_EQ(rc, 1);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 0);
    ASSERT_EQ(second, 1);
}

UTEST_F(Timekeeper, Accumulate1999ms) {
    uint8_t hour, minute, second;
    int rc;

    rc = timekeeperAccumulate(utest_fixture->tk, 1999);
    ASSERT_EQ(rc, 1);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 0);
    ASSERT_EQ(second, 1);
}

UTEST_F(Timekeeper, AccumulateMinute10Times100ms) {
    uint8_t hour, minute, second;

    for (int i = 0; i < 10; i++) {
        timekeeperAccumulate(utest_fixture->tk, 100);
    }

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 0);
    ASSERT_EQ(second, 1);
}

UTEST_F(Timekeeper, AccumulateMinuteWrap) {
    uint8_t hour, minute, second;

    timekeeperSet(utest_fixture->tk, 0, 0, 59);

    timekeeperAccumulate(utest_fixture->tk, 1000);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 1);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, AccumulateHourWrap) {
    uint8_t hour, minute, second;

    timekeeperSet(utest_fixture->tk, 0, 59, 59);

    timekeeperAccumulate(utest_fixture->tk, 1000);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 1);
    ASSERT_EQ(minute, 0);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, AccumulateDayWrap) {
    uint8_t hour, minute, second;

    timekeeperSet(utest_fixture->tk, 23, 59, 59);

    timekeeperAccumulate(utest_fixture->tk, 1000);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 0);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, SetFlag) {
    timekeeperSet(utest_fixture->tk, 12, 34, 56);
    int rc = timekeeperAccumulate(utest_fixture->tk, 0);
    ASSERT_EQ(rc, -3);
}

UTEST_F(Timekeeper, Init) {
    uint8_t hour, minute, second;
    timekeeperInit(utest_fixture->tk);

    timekeeperGet(utest_fixture->tk, &hour, &minute, &second);
    int rc = timekeeperAccumulate(utest_fixture->tk, 999);
    ASSERT_EQ(rc, 0);

    ASSERT_EQ(hour, 0);
    ASSERT_EQ(minute, 0);
    ASSERT_EQ(second, 0);
}

UTEST_F(Timekeeper, GetterNULL) {
    timekeeperGet(utest_fixture->tk, NULL, NULL, NULL);
}
