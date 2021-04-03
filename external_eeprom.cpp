#include <Arduino.h>
#include <Wire.h>
#include "eeprom_access.h"

static int extMemRead(void *user, uint16_t address, uint8_t *value);
static int extMemWrite(void *user, uint16_t address, uint8_t value);
static int extMemRead32(void *user, uint16_t address, uint8_t *value);
static int extMemWrite32(void *user, uint16_t address, uint8_t const *value);

static eeprom_access_t extmemAccess {
    .user = NULL,
    .read = extMemRead,
    .write = extMemWrite,
    .read32 = extMemRead32,
    .write32 = extMemWrite32,
};


void extmemInit() {
    Wire.begin();
    Wire.setClock(100000);
}

eeprom_access_t *extmemGetAccess() {
    return &extmemAccess;
}

#define EXTMEM_DEV_ADDR(page) (0b1010000 | (page))

static int extMemReadN(uint16_t address, uint8_t count, uint8_t *buf) {
    uint8_t addr = (uint8_t)(address & 0xFF);
    uint8_t page = (uint8_t)((address >> 8) & 1);

    uint8_t devAddr = EXTMEM_DEV_ADDR(page);

    Wire.beginTransmission(devAddr);
    Wire.write(addr);
    Wire.endTransmission();

    Wire.requestFrom(devAddr, count);

    for(uint8_t i = 0; i < count; i++) {
        buf[i] = Wire.read();
    }

    return 0;
}

static int extMemWriteN(uint16_t address, uint8_t count, uint8_t const *buf) {
    uint8_t addr = (uint8_t)(address & 0xFF);
    uint8_t page = (uint8_t)((address >> 8) & 1);

    uint8_t devAddr = EXTMEM_DEV_ADDR(page);
    
    Wire.beginTransmission(devAddr);
    Wire.write(addr);

    for(uint8_t i = 0; i < count; i++) {
        Wire.write(buf[i]);
    }

    Wire.endTransmission();

    return 0;
}

static int extMemRead(void *user, uint16_t address, uint8_t *value) {
    return extMemReadN(address, 1, value);
}

static int extMemWrite(void *user, uint16_t address, uint8_t value) {
    return extMemWriteN(address, 1, &value);
}

static int extMemRead32(void *user, uint16_t address, uint8_t *value) {
    return extMemReadN(address, 4, value);
}

static int extMemWrite32(void *user, uint16_t address, uint8_t const *value) {
    return extMemWriteN(address, 4, value);
}
