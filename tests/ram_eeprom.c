#include "ram_eeprom.h"
#include <string.h>

static int eeprom_read(void *user, uint16_t address, uint8_t count, uint8_t nmemb, uint8_t *value) {
    ram_eeprom_buffer_t *state = (ram_eeprom_buffer_t *)user;

    unsigned total = count * nmemb;

    if (address + total > sizeof(state->buffers)) {
        return 1;
    }

    for (unsigned i = 0; i < total; i++) {
        value[i] = state->raw[address + i];
    }

    return 0;
}

static int eeprom_write(void *user, uint16_t address, uint8_t count, uint8_t nmemb, uint8_t const *value) {
    ram_eeprom_buffer_t *state = (ram_eeprom_buffer_t *)user;

    unsigned total = count * nmemb;

    if (address + total > sizeof(state->buffers)) {
        return 1;
    }

    for (unsigned i = 0; i < total; i++) {
        state->raw[address + i] = value[i];
    }

    return 0;
}

void ramEepromFillDescriptor(eeprom_access_t *access, ram_eeprom_buffer_t *buffer) {
    access->user = buffer;
    access->read = eeprom_read;
    access->write = eeprom_write;
}

void ramEepromClear(ram_eeprom_buffer_t *buffer) {
    memset(buffer, 0, sizeof(ram_eeprom_buffer_t));
}
