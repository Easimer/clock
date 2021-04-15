#ifndef CLOCK_TESTS_RAM_EEPROM_H
#define CLOCK_TESTS_RAM_EEPROM_H

#include <stdint.h>
#include "eeprom_access.h"

#define EEPROM_ELEMENTS (16)

#pragma pack(push, 1)
typedef struct ram_eeprom_buffer {
    union {
        struct {
            uint8_t signature;
            uint8_t status[EEPROM_ELEMENTS];
            uint32_t parameter[EEPROM_ELEMENTS];
        } buffers;
        uint8_t raw[1];
    };
} ram_eeprom_buffer_t;
#pragma pack(pop)

void ramEepromFillDescriptor(eeprom_access_t *access, ram_eeprom_buffer_t *buffer);
void ramEepromClear(ram_eeprom_buffer_t *buffer);

#endif /* CLOCK_TESTS_RAM_EEPROM_H */
