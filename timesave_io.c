#include "timesave_io.h"
#include <stddef.h>
#include <string.h>

#define MAKE_SIGNATURE(version) (0xC0 | (version & 0xF))
#define VERSION 0

#define SIGNATURE MAKE_SIGNATURE(VERSION)

static uint8_t calculateChecksum(uint8_t b0, uint8_t b1, uint8_t b2) {
    uint16_t sum = 0;
    sum += b0;
    sum += b1;
    sum += b2;
    
    while((sum >> 8) > 0) {
        sum = (sum & 0xFF) + (sum >> 8);
    }
    
    return ~sum;
}

timesave_io_status_t restoreTime(timekeeper_t *tk, eeprom_access_t *mem) {
    timesave_io_status_t ret;
    uint8_t buf[4];

    // We try eight times
    for(uint8_t i = 0; i < 8; i++) {
        mem->read32(mem->user, 0, buf);

        if(buf[0] != SIGNATURE) {
            ret = ETIMESAVE_IO_BADSIG;
            continue;
        }

        uint8_t checksumCalculated = calculateChecksum(buf[0], buf[1], buf[2]);

        if(checksumCalculated != buf[3]) {
            ret = ETIMESAVE_IO_BADCHK;
            continue;
        }

        timekeeperSet(tk, buf[1], buf[2]);
        return ETIMESAVE_IO_OK;
    }

    return ret;
}

timesave_io_status_t saveTime(timekeeper_t const *tk, eeprom_access_t *mem) {
    uint8_t buf[4];

    buf[0] = SIGNATURE;
    timekeeperGet(tk, &buf[1], &buf[2], NULL);
    buf[3] = calculateChecksum(SIGNATURE, buf[1], buf[2]);

    for(uint8_t attempt = 0; attempt < 32; attempt++) {
        mem->write32(mem->user, 0, buf);
        delay(64);

        uint8_t readbackBuf[4];
        mem->read32(mem->user, 0, readbackBuf);

        if(memcmp(buf, readbackBuf, 4) == 0) {
            return ETIMESAVE_IO_OK;
        }

        delay(64);
    }

    return ETIMESAVE_IO_WRITE_FAILURE;
}