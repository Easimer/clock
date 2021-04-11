#include <Arduino.h>
#include <Wire.h>
#include "eeprom_access.h"

// Driver for the 'PCF8594C-2 512 x 8-bit CMOS EEPROM with I2C-bus interface"

static int extMemRead(void *user, uint16_t address, uint8_t size, uint8_t nmemb, uint8_t *ptr);
static int extMemWrite(void *user, uint16_t address, uint8_t size, uint8_t nmemb, uint8_t const *ptr);

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

static int extMemRead(void *user, uint16_t address, uint8_t size, uint8_t nmemb, uint8_t *ptr) {
    if(nmemb > 1 || size > 8) {
        // TODO: proper implementation
        return -1;
    }

    uint8_t addr = (uint8_t)(address & 0xFF);
    uint8_t page = (uint8_t)((address >> 8) & 1);

    uint8_t devAddr = EXTMEM_DEV_ADDR(page);

    Wire.beginTransmission(devAddr);
    Wire.write(addr);
    Wire.endTransmission();

    for(uint8_t elem = 0; elem < nmemb; elem++) {
        Wire.requestFrom(devAddr, size);
        for(uint8_t byte = 0; byte < size; byte++) {
            *(ptr++) = Wire.read();
        }
    }

    return 0;
}

static int extMemWrite(void *user, uint16_t address, uint8_t size, uint8_t nmemb, uint8_t const *ptr) {
    if(nmemb > 1 || size > 8) {
        // TODO: proper implementation
        return -1;
    }

    uint8_t addr = (uint8_t)(address & 0xFF);
    uint8_t page = (uint8_t)((address >> 8) & 1);

    uint8_t devAddr = EXTMEM_DEV_ADDR(page);
    
    Wire.beginTransmission(devAddr);
    Wire.write(addr);

    for(uint8_t elem = 0; elem < nmemb; elem++) {
        for(uint8_t byte = 0; byte < size; byte++) {
            Wire.write(*ptr++);
        }
    }

    Wire.endTransmission();

    return 0;
}

/*
 * ISSUES WITH THIS CODE:
 * 
 * There is no logic that handles reads/writes bigger than 8 bytes
 * and/or 1 elements. Since we can't send more than 8 bytes in a
 * single transmission we would have to increment the destination
 * address every 8 bytes.
 * 
 * But then we'd have to watch out for the case when reading or
 * writing on a (256-byte) page boundary, since the address wraps in
 * a funny way there when the chip auto-increments it
 * (255->0, 511->256).
 * 
 * We don't really need these features yet, because we only write
 * either single bytes (signature, status buffer) or at most eight
 * bytes (parameter buffer).
 * Exception is when we zeroing out the status buffer, but there is
 * a workaround in place for that.
 */