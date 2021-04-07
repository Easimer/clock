#include "timesave_io.h"
#include <stddef.h>
#include <string.h>

#define MAKE_SIGNATURE(version) (0xC0 | (version & 0xF))
#define VERSION 1

#define SIGNATURE MAKE_SIGNATURE(VERSION)

typedef struct entry {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t checksum;
} entry_t;

static uint8_t calculateChecksum(void const *buf, uint8_t size) {
    uint16_t sum = 0;
    uint8_t const *src = (uint8_t const *)buf;

    for (uint8_t i = 0; i < size; i++) {
        sum += src[i];
    }
    
    while((sum >> 8) > 0) {
        sum = (sum & 0xFF) + (sum >> 8);
    }
    
    return ~sum;
}

timesave_io_status_t restoreTime(timekeeper_t *tk, timesave_io_config_t *cfg) {
    if (tk == NULL || cfg == NULL) {
        return ETIMESAVE_IO_INVALID_ARG;
    }

    uint8_t bufSignature = 0;
    if (cfg->emhe.access->read(cfg->emhe.access->user, cfg->startAddress, &bufSignature) != 0 || bufSignature != SIGNATURE) {
        return ETIMESAVE_IO_BADSIG;
    }

    entry_t entry;

    if (emheRead(&cfg->emhe, &entry) != 0) {
        return ETIMESAVE_IO_READ_FAILURE;
    }

    uint8_t checksumRead = entry.checksum;
    entry.checksum = 0;
    uint8_t checksumCalculated = calculateChecksum(&entry, sizeof(entry));

    if (checksumCalculated != checksumRead) {
        return ETIMESAVE_IO_BADCHK;
    }

    timekeeperSet(tk, entry.hour, entry.minute, entry.second);

    return ETIMESAVE_IO_OK;
}

timesave_io_status_t saveTime(timekeeper_t const *tk, timesave_io_config_t *cfg) {
    timesave_io_status_t ret = ETIMESAVE_IO_OK;
    uint8_t bufSignature = 0;
    entry_t entry;

    if (tk == NULL || cfg == NULL) {
        return ETIMESAVE_IO_INVALID_ARG;
    }

    if (cfg->emhe.access->read(cfg->emhe.access->user, cfg->startAddress, &bufSignature) != 0 || bufSignature != SIGNATURE) {
        // Bad signature, try to reset the data storage
        if (cfg->emhe.access->write(cfg->emhe.access->user, cfg->startAddress, SIGNATURE) != 0) {
            return ETIMESAVE_IO_WRITE_FAILURE;
        }

        cfg->emhe.flags = EMHE_F_RESET;
        if (emheInit(&cfg->emhe) != 0) {
            return ETIMESAVE_IO_WRITE_FAILURE;
        }

        ret = ETIMESAVE_IO_ERASED;
    }

    if (cfg->emhe.access->read(cfg->emhe.access->user, cfg->startAddress, &bufSignature) != 0 || bufSignature != SIGNATURE) {
        return ETIMESAVE_IO_BADSIG;
    }

    timekeeperGet(tk, &entry.hour, &entry.minute, &entry.second);
    entry.checksum = 0;
    entry.checksum = calculateChecksum(&entry, sizeof(entry));

    if (emheWrite(&cfg->emhe, &entry) != 0) {
        return ETIMESAVE_IO_WRITE_FAILURE;
    }

    return ret;
}

timesave_io_status_t fillTimesaveConfig(timesave_io_config_t *cfg, eeprom_access_t *access, uint16_t startAddress) {
    if (cfg == NULL || access == NULL) {
        return ETIMESAVE_IO_INVALID_ARG;
    }

    cfg->startAddress = startAddress;
    cfg->emhe.access = access;
    // One extra byte for the signature
    cfg->emhe.address = startAddress + 1;
    cfg->emhe.elementCount = 16;
    cfg->emhe.elementSize = sizeof(entry_t);
    cfg->emhe.flags = EMHE_F_NONE;

    if (emheInit(&cfg->emhe) != 0) {
        return ETIMESAVE_IO_INVALID_ARG;
    }

    return ETIMESAVE_IO_OK;
}