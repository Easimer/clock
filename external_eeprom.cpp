#include <Arduino.h>
#include <Wire.h>
#include <stddef.h>
#include "eeprom_access.h"

static int extMemRead(void *user, uint16_t address, uint8_t *value);
static int extMemWrite(void *user, uint16_t address, uint8_t value);

static eeprom_access_t extmemAccess {
    .user = NULL,
    .read = extMemRead,
    .write = extMemWrite,
};


void extmemInit() {
    Wire.begin();
    Wire.setClock(100000);
}

eeprom_access_t *extmemGetAccess() {
    return &extmemAccess;
}

#define EXTMEM_DEV_ADDR(page) (0b1010000 | (page))

static int extMemRead(void *user, uint16_t address, uint8_t *value) {
    uint8_t addr = (uint8_t)(address & 0xFF);
    uint8_t page = (uint8_t)((address >> 8) & 1);

    *value = 0;

    uint8_t devAddr = EXTMEM_DEV_ADDR(page);
    
    Wire.beginTransmission(devAddr);
    Wire.write(addr);
    Wire.endTransmission();

    Wire.requestFrom(devAddr, 1);
    *value = Wire.read();

    return 0;
}

static int extMemWrite(void *user, uint16_t address, uint8_t value) {
    uint8_t addr = (uint8_t)(address & 0xFF);
    uint8_t page = (uint8_t)((address >> 8) & 1);

    uint8_t devAddr = EXTMEM_DEV_ADDR(page);
    Wire.beginTransmission(devAddr);
    Wire.write(addr);
    Wire.write(value);
    Wire.endTransmission();

    return 0;
}