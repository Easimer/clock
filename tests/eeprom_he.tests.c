#include "utest.h"

#include "eeprom_he.h"

UTEST_MAIN();

#define EEPROM_ELEMENTS (16)

struct EepromHE {
	eeprom_access_t access;
	
	union {
		struct {
			uint8_t status[EEPROM_ELEMENTS];
			uint32_t parameter[EEPROM_ELEMENTS];
		} buffers;
		uint8_t raw[1];
	};

	emhe_descriptor_t descriptor;
};

static int eeprom_read(void *user, uint16_t address, uint8_t *value) {
	struct EepromHE *state = (struct EepromHE *)user;

	if (address >= sizeof(state->buffers)) {
		return 1;
	}

	value[0] = state->raw[address];

	return 0;
}

static int eeprom_write(void *user, uint16_t address, uint8_t value) {
	struct EepromHE *state = (struct EepromHE *)user;

	if (address >= sizeof(state->buffers)) {
		return 1;
	}

	state->raw[address] = value;

	return 0;
}

static int eeprom_read32(void *user, uint16_t address, uint8_t *value) {
	struct EepromHE *state = (struct EepromHE *)user;

	if (address + 3 >= sizeof(state->buffers)) {
		return 1;
	}

	for (int i = 0; i < 4; i++) {
		value[i] = state->raw[address + i];
	}

	return 0;
}

static int eeprom_write32(void *user, uint16_t address, uint8_t const *value) {
	struct EepromHE *state = (struct EepromHE *)user;

	if (address + 3 >= sizeof(state->buffers)) {
		return 1;
	}

	for (int i = 0; i < 4; i++) {
		state->raw[address + i] = value[i];
	}

	return 0;
}

UTEST_F_SETUP(EepromHE) {
	utest_fixture->access.user = utest_fixture;
	utest_fixture->access.read = eeprom_read;
	utest_fixture->access.write = eeprom_write;
	utest_fixture->access.read32 = eeprom_read32;
	utest_fixture->access.write32 = eeprom_write32;

	utest_fixture->descriptor.access = &utest_fixture->access;
	utest_fixture->descriptor.address = 0;
	utest_fixture->descriptor.elementCount = EEPROM_ELEMENTS;
	utest_fixture->descriptor.elementSize = sizeof(uint32_t);
	utest_fixture->descriptor.flags = EMHE_F_NONE;
}

UTEST_F_TEARDOWN(EepromHE) {
}

UTEST_F(EepromHE, Init) {
	int rc;

	memset(&utest_fixture->buffers, 0, sizeof(utest_fixture->buffers));

	utest_fixture->descriptor.flags = EMHE_F_RESET;
	rc = emheInit(&utest_fixture->descriptor);

	ASSERT_EQ(rc, 0);
	ASSERT_EQ(emheGetPointer(&utest_fixture->descriptor), 0);
	ASSERT_EQ(utest_fixture->descriptor.flags & EMHE_F_INITIALIZED, EMHE_F_INITIALIZED);
}

UTEST_F(EepromHE, ReadFirst) {
	int rc;
	uint32_t buf[1];

	uint32_t const expected = 0xDEADBEEF;

	memset(&utest_fixture->buffers, 0, sizeof(utest_fixture->buffers));

	utest_fixture->buffers.parameter[0] = expected;

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

	memset(&utest_fixture->buffers, 0, sizeof(utest_fixture->buffers));

	for (int i = 0; i < 10; i++) {
		utest_fixture->buffers.status[i] = i;
	}

	utest_fixture->buffers.parameter[9] = expected;

	rc = emheInit(&utest_fixture->descriptor);
	ASSERT_EQ(rc, 0);

	rc = emheRead(&utest_fixture->descriptor, buf);
	ASSERT_EQ(rc, 0);

	ASSERT_EQ(buf[0], expected);
}

UTEST_F(EepromHE, WriteFirst) {
	int rc;
	uint32_t const expected = 0xDEADBEEF;

	memset(&utest_fixture->buffers, 0, sizeof(utest_fixture->buffers));

	rc = emheInit(&utest_fixture->descriptor);
	ASSERT_EQ(rc, 0);

	rc = emheWrite(&utest_fixture->descriptor, &expected);
	ASSERT_EQ(rc, 0);

	ASSERT_EQ(utest_fixture->buffers.parameter[1], expected);
	ASSERT_EQ(emheGetPointer(&utest_fixture->descriptor), 1);
}

UTEST_F(EepromHE, WriteNth) {
	int rc;
	uint32_t const expected = 0xDEADBEEF;

	memset(&utest_fixture->buffers, 0, sizeof(utest_fixture->buffers));

	for (int i = 0; i < 10; i++) {
		utest_fixture->buffers.status[i] = i;
	}

	rc = emheInit(&utest_fixture->descriptor);
	ASSERT_EQ(rc, 0);

	rc = emheWrite(&utest_fixture->descriptor, &expected);
	ASSERT_EQ(rc, 0);

	ASSERT_EQ(utest_fixture->buffers.status[10], 10);
	ASSERT_EQ(utest_fixture->buffers.parameter[10], expected);
	ASSERT_EQ(emheGetPointer(&utest_fixture->descriptor), 10);
}

UTEST_F(EepromHE, ReadWrapping) {
	int rc;
	uint32_t const expected = 0xDEADBEEF;
	uint32_t buf[1];

	memset(&utest_fixture->buffers, 0, sizeof(utest_fixture->buffers));

	for (int i = 0; i < 16; i++) {
		utest_fixture->buffers.status[i] = i;
	}
	utest_fixture->buffers.parameter[15] = expected;

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

	memset(&utest_fixture->buffers, 0, sizeof(utest_fixture->buffers));

	for (int i = 0; i < 16; i++) {
		utest_fixture->buffers.status[i] = i;
	}

	rc = emheInit(&utest_fixture->descriptor);
	ASSERT_EQ(rc, 0);

	rc = emheWrite(&utest_fixture->descriptor, &expected);
	ASSERT_EQ(rc, 0);

	ASSERT_EQ(utest_fixture->buffers.status[0], 16);
	ASSERT_EQ(emheGetPointer(&utest_fixture->descriptor), 0);
	ASSERT_EQ(utest_fixture->buffers.parameter[0], expected);
}

UTEST_F(EepromHE, Sequence) {
	int rc;
	uint32_t rd;

	memset(&utest_fixture->buffers, 0, sizeof(utest_fixture->buffers));

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