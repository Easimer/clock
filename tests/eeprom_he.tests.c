#include "utest.h"

#include "eeprom_he.h"
#include "ram_eeprom.h"

UTEST_MAIN();

#define EEPROM_ELEMENTS (16)

struct EepromHE {
    eeprom_access_t access;
    ram_eeprom_buffer_t memory;
    emhe_descriptor_t descriptor;
};

UTEST_F_SETUP(EepromHE) {
    ramEepromFillDescriptor(&utest_fixture->access, &utest_fixture->memory);
    ramEepromClear(&utest_fixture->memory);

    utest_fixture->descriptor.access = &utest_fixture->access;
    uint16_t statusBufferOffset = (uint16_t)&(((ram_eeprom_buffer_t *)NULL)->buffers.status);
    utest_fixture->descriptor.address = statusBufferOffset;
    utest_fixture->descriptor.elementCount = EEPROM_ELEMENTS;
    utest_fixture->descriptor.elementSize = sizeof(uint32_t);
    utest_fixture->descriptor.flags = EMHE_F_NONE;
}

UTEST_F_TEARDOWN(EepromHE) {
}

UTEST_F(EepromHE, Init) {
    int rc;

    utest_fixture->descriptor.flags = EMHE_F_RESET;
    rc = emheInit(&utest_fixture->descriptor);

    ASSERT_EQ(rc, 0);
    ASSERT_EQ(emheGetPointer(&utest_fixture->descriptor), 0);
    ASSERT_EQ((utest_fixture->descriptor.flags & EMHE_F_INITIALIZED), EMHE_F_INITIALIZED);
}

UTEST_F(EepromHE, ReadFirst) {
    int rc;
    uint32_t buf[1];

    uint32_t const expected = 0xDEADBEEF;

    utest_fixture->memory.buffers.parameter[0] = expected;

    rc = emheInit(&utest_fixture->descriptor);
    ASSERT_EQ(rc, 0);

    rc = emheRead(&utest_fixture->descriptor, buf);
    ASSERT_EQ(rc, 0);

    ASSERT_EQ(buf[0], expected);
}

UTEST_F(EepromHE, ReadNth) {
    int rc;
    uint32_t buf[1];

    uint32_t const expected = 0xDEADBEEF;

    for (int i = 0; i < 10; i++) {
        utest_fixture->memory.buffers.status[i] = i;
    }

    utest_fixture->memory.buffers.parameter[9] = expected;

    rc = emheInit(&utest_fixture->descriptor);
    ASSERT_EQ(rc, 0);

    rc = emheRead(&utest_fixture->descriptor, buf);
    ASSERT_EQ(rc, 0);

    ASSERT_EQ(buf[0], expected);
}

UTEST_F(EepromHE, WriteFirst) {
    int rc;
    uint32_t const expected = 0xDEADBEEF;

    rc = emheInit(&utest_fixture->descriptor);
    ASSERT_EQ(rc, 0);

    rc = emheWrite(&utest_fixture->descriptor, &expected);
    ASSERT_EQ(rc, 0);

    ASSERT_EQ(utest_fixture->memory.buffers.parameter[1], expected);
    ASSERT_EQ(emheGetPointer(&utest_fixture->descriptor), 1);
}

UTEST_F(EepromHE, WriteNth) {
    int rc;
    uint32_t const expected = 0xDEADBEEF;

    for (int i = 0; i < 10; i++) {
        utest_fixture->memory.buffers.status[i] = i;
    }

    rc = emheInit(&utest_fixture->descriptor);
    ASSERT_EQ(rc, 0);

    rc = emheWrite(&utest_fixture->descriptor, &expected);
    ASSERT_EQ(rc, 0);

    ASSERT_EQ(utest_fixture->memory.buffers.status[10], 10);
    ASSERT_EQ(utest_fixture->memory.buffers.parameter[10], expected);
    ASSERT_EQ(emheGetPointer(&utest_fixture->descriptor), 10);
}

UTEST_F(EepromHE, ReadWrapping) {
    int rc;
    uint32_t const expected = 0xDEADBEEF;
    uint32_t buf[1];

    for (int i = 0; i < 16; i++) {
        utest_fixture->memory.buffers.status[i] = i;
    }
    utest_fixture->memory.buffers.parameter[15] = expected;

    rc = emheInit(&utest_fixture->descriptor);
    ASSERT_EQ(rc, 0);

    rc = emheRead(&utest_fixture->descriptor, buf);
    ASSERT_EQ(rc, 0);

    ASSERT_EQ(emheGetPointer(&utest_fixture->descriptor), 15);
    ASSERT_EQ(buf[0], expected);
}

UTEST_F(EepromHE, WriteWrapping) {
    int rc;
    uint32_t const expected = 0xDEADBEEF;

    for (int i = 0; i < 16; i++) {
        utest_fixture->memory.buffers.status[i] = i;
    }

    rc = emheInit(&utest_fixture->descriptor);
    ASSERT_EQ(rc, 0);

    rc = emheWrite(&utest_fixture->descriptor, &expected);
    ASSERT_EQ(rc, 0);

    ASSERT_EQ(utest_fixture->memory.buffers.status[0], 16);
    ASSERT_EQ(emheGetPointer(&utest_fixture->descriptor), 0);
    ASSERT_EQ(utest_fixture->memory.buffers.parameter[0], expected);
}

UTEST_F(EepromHE, Sequence) {
    int rc;
    uint32_t rd;

    for (uint32_t i = 0; i < 255; i++) {
        utest_fixture->descriptor.flags = EMHE_F_NONE;
        rc = emheInit(&utest_fixture->descriptor);
        ASSERT_EQ(rc, 0);

        rc = emheWrite(&utest_fixture->descriptor, &i);
        ASSERT_EQ(rc, 0);
        uint8_t ptr = emheGetPointer(&utest_fixture->descriptor);
        
        utest_fixture->descriptor.flags = EMHE_F_NONE;
        rc = emheInit(&utest_fixture->descriptor);
        ASSERT_EQ(rc, 0);
        ASSERT_EQ(emheGetPointer(&utest_fixture->descriptor), ptr);

        rc = emheRead(&utest_fixture->descriptor, &rd);
        ASSERT_EQ(rd, i);
    }
}
