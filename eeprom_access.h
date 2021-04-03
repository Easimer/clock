#ifndef CLOCK_EEPROM_ACCESS
#define CLOCK_EEPROM_ACCESS

#include <stdint.h>

typedef int (*eeprom_access_read_t)(void *user, uint16_t address, uint8_t *value);
typedef int (*eeprom_access_write_t)(void *user, uint16_t address, uint8_t value);

typedef int (*eeprom_access_read32_t)(void *user, uint16_t address, uint8_t *value);
typedef int (*eeprom_access_write32_t)(void *user, uint16_t address, uint8_t const *value);

typedef struct eeprom_access {
    void *user;
    eeprom_access_read_t read;
    eeprom_access_write_t write;

    eeprom_access_read32_t read32;
    eeprom_access_write32_t write32;
} eeprom_access_t;

void extmemInit();
eeprom_access_t *extmemGetAccess();

#endif /* CLOCK_EEPROM_ACCESS */