#ifndef CLOCK_EEPROM_HE_H
#define CLOCK_EEPROM_HE_H

// EEPROM High Endurance

#include <stdint.h>
#include "eeprom_access.h"

typedef enum emhe_flags {
	EMHE_F_NONE = 0,
	EMHE_F_INITIALIZED = (1 << 0),
	EMHE_F_RESET = (1 << 1),
} emhe_flags_t;

typedef struct emhe_descriptor {
	eeprom_access_t *access;
	uint8_t elementSize;
	uint8_t elementCount;
	uint16_t address;
	uint8_t flags;

	uint8_t pointer;
} emhe_descriptor_t;

#define EMHE_GET_BUFFER_SIZE_REQUIRED(elementSize, elementCount) ((elementSize) * (elementCount) + (elementCount) * sizeof(uint8_t))

#ifdef __cplusplus
extern "C" {
#endif

int emheInit(emhe_descriptor_t *descriptor);
int emheRead(emhe_descriptor_t *descriptor, void *buffer);
int emheWrite(emhe_descriptor_t *descriptor, void const *buffer);
uint8_t emheGetPointer(emhe_descriptor_t const *descriptor);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_EEPROM_HE_H */