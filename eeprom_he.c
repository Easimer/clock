#include "eeprom_he.h"
#include <stdint.h>
#include <stddef.h>

static uint16_t getStatusBufferAddress(emhe_descriptor_t const *D, uint8_t idx) {
	return D->address + idx;
}

static uint16_t getParameterBufferAddress(emhe_descriptor_t const *D, uint8_t idx) {
	return D->address + D->elementCount * sizeof(uint8_t) + D->elementSize * idx;
}

static int findAddress(emhe_descriptor_t const *D, uint8_t *outAddr) {
	int rc;
	uint8_t cur, next;
	uint16_t addr = getStatusBufferAddress(D, 0);

	uint16_t lastAddr = addr + D->elementCount;

	do {
		rc = D->access->read(D->access->user, addr, 1, 1, &cur);
		if (rc != 0) { return rc; }

		addr++;
		if (addr == lastAddr) {
			break;
		}

		rc = D->access->read(D->access->user, addr, 1, 1, &next);
		if (rc != 0) { return rc; }
	} while (next == cur + 1);

	*outAddr = addr - getStatusBufferAddress(D, 0) - 1;
	
	return 0;
}

static int zeroStatusBuffer(emhe_descriptor_t *D) {
	int rc;
	uint8_t const zero[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	uint8_t remaining = D->elementCount;
	uint16_t addr = getStatusBufferAddress(D, 0);

	while(remaining >= 8) {
		rc = D->access->write(D->access->user, addr, 8, 1, zero);
		addr += 8;
		remaining -= 8;
	}
	
	while(remaining > 0) {
		rc = D->access->write(D->access->user, addr, 1, 1, zero);
		addr++;
		remaining--;
	}

	return 0;
}

int emheInit(emhe_descriptor_t *D) {
	if (D == NULL) {
		return 1;
	}

	if (D->flags & EMHE_F_INITIALIZED) {
		return 0;
	}

	if (D->flags & EMHE_F_RESET) {
		D->pointer = 0;
		if (zeroStatusBuffer(D) != 0) {
			return 1;
		}
	} else {
		if (findAddress(D, &D->pointer) != 0) {
			return 1;
		}
	}

	D->flags |= EMHE_F_INITIALIZED;

	return 0;
}

int emheRead(emhe_descriptor_t *D, void *buffer) {
	if (D == NULL) {
		return 1;
	}

	int rc;
	uint16_t addr = getParameterBufferAddress(D, D->pointer);
	uint8_t remaining = D->elementSize;
	uint8_t *dst = (uint8_t *)buffer;

	rc = D->access->read(D->access->user, addr, D->elementSize, 1, dst);

	return 0;
}

int emheWrite(emhe_descriptor_t *D, void const *buffer) {
	if (D == NULL) {
		return 1;
	}

	int rc;
	uint8_t status;
	uint16_t sAddr = getStatusBufferAddress(D, D->pointer);
	rc = D->access->read(D->access, sAddr, 1, 1, &status);
	if (rc != 0) return rc;

	if (D->pointer + 1 == D->elementCount) {
		D->pointer = 0;
	} else {
		D->pointer++;
	}

	uint16_t pAddr = getParameterBufferAddress(D, D->pointer);
	sAddr = getStatusBufferAddress(D, D->pointer);

	uint8_t remaining = D->elementSize;
	uint8_t const *src = (uint8_t const *)buffer;

	rc = D->access->write(D->access->user, pAddr, D->elementSize, 1, src);
	if (rc != 0) return rc;

	status++;
	rc = D->access->write(D->access->user, sAddr, 1, 1, &status);
	if (rc != 0) return rc;

	return 0;
}

uint8_t emheGetPointer(emhe_descriptor_t const *descriptor) {
	return descriptor->pointer;
}