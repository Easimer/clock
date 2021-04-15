#include "utest.h"
#include "config.h"
#include "timesave_io.h"
#include "ram_eeprom.h"

typedef struct TimesaveIO {
    eeprom_access_t access;
    ram_eeprom_buffer_t memory;
    timesave_io_config_t config;
} state_t;

UTEST_F_SETUP(TimesaveIO) {
    timesave_io_status_t rc;
    ramEepromFillDescriptor(&utest_fixture->access, &utest_fixture->memory);
    ramEepromClear(&utest_fixture->memory);

    uint16_t startAddress = 0;
    rc = fillTimesaveConfig(&utest_fixture->config, &utest_fixture->access, startAddress);
    ASSERT_EQ(rc, ETIMESAVE_IO_OK);
}

UTEST_F_TEARDOWN(TimesaveIO) {
}

UTEST_F(TimesaveIO, InitializesEeprom) {
    timesave_io_status_t rc;
    TIMEKEEPER_CREATE_ON_STACK(tkTime);
    rc = saveTime(tkTime, &utest_fixture->config);
    ASSERT_EQ(rc, ETIMESAVE_IO_ERASED);
}

UTEST_F(TimesaveIO, SecondSaveDoesntReinitializeEeprom) {
    timesave_io_status_t rc;
    TIMEKEEPER_CREATE_ON_STACK(tkTime);
    rc = saveTime(tkTime, &utest_fixture->config);
    ASSERT_EQ(rc, ETIMESAVE_IO_ERASED);
    rc = saveTime(tkTime, &utest_fixture->config);
    ASSERT_EQ(rc, ETIMESAVE_IO_OK);
}

UTEST_F(TimesaveIO, SaveRestore) {
    timesave_io_status_t rc;
    uint8_t hour, minute, second;
    uint8_t expectedHour = 12, expectedMinute = 34, expectedSecond = 56;
    TIMEKEEPER_CREATE_ON_STACK(tkTime);

    timekeeperSet(tkTime, expectedHour, expectedMinute, expectedSecond);

    rc = saveTime(tkTime, &utest_fixture->config);
    ASSERT_EQ(rc, ETIMESAVE_IO_ERASED);

    rc = restoreTime(tkTime, &utest_fixture->config);
    ASSERT_EQ(rc, ETIMESAVE_IO_OK);

    timekeeperGet(tkTime, &hour, &minute, &second);

    ASSERT_EQ(hour, expectedHour);
    ASSERT_EQ(minute, expectedMinute);
    ASSERT_EQ(second, expectedSecond);

    ASSERT_EQ(utest_fixture->config.emhe.pointer, 1);
}

UTEST_F(TimesaveIO, RestoreFailsOnCleanEeprom) {
    timesave_io_status_t rc;
    TIMEKEEPER_CREATE_ON_STACK(tkTime);

    rc = restoreTime(tkTime, &utest_fixture->config);
    ASSERT_EQ(rc, ETIMESAVE_IO_BADSIG);
}

UTEST_F(TimesaveIO, RestoreNullTimekeeper) {
    timesave_io_status_t rc;

    rc = restoreTime(NULL, &utest_fixture->config);
    ASSERT_EQ(rc, ETIMESAVE_IO_INVALID_ARG);
}

UTEST_F(TimesaveIO, RestoreNullConfig) {
    timesave_io_status_t rc;
    TIMEKEEPER_CREATE_ON_STACK(tkTime);

    rc = restoreTime(tkTime, NULL);
    ASSERT_EQ(rc, ETIMESAVE_IO_INVALID_ARG);
}

UTEST_F(TimesaveIO, BadChecksum) {
    timesave_io_status_t rc;
    TIMEKEEPER_CREATE_ON_STACK(tkTime);

    rc = saveTime(tkTime, &utest_fixture->config);
    ASSERT_EQ(rc, ETIMESAVE_IO_ERASED);

    // Add one to each value in the parameter buffer to make the
    // checksum invalid
    for (int i = 0; i < EEPROM_ELEMENTS; i++) {
        utest_fixture->memory.buffers.parameter[i]++;
    }

    rc = restoreTime(tkTime, &utest_fixture->config);
    ASSERT_EQ(rc, ETIMESAVE_IO_BADCHK);
}
