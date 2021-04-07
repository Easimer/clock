#ifndef CLOCK_TIMESAVE_IO_H
#define CLOCK_TIMESAVE_IO_H

#include "eeprom_he.h"
#include "timekeeper.h"

typedef enum timesave_io_status {
    ETIMESAVE_IO_OK = 0,
    ETIMESAVE_IO_BADSIG,
    ETIMESAVE_IO_BADCHK,
    ETIMESAVE_IO_READ_FAILURE,
    ETIMESAVE_IO_WRITE_FAILURE,
    ETIMESAVE_IO_INVALID_ARG,
    ETIMESAVE_IO_ERASED,
} timesave_io_status_t;

typedef struct timesave_io_config {
    uint16_t startAddress;
    emhe_descriptor_t emhe;
} timesave_io_config_t;

#ifdef __cplusplus
extern "C" {
#endif

timesave_io_status_t fillTimesaveConfig(timesave_io_config_t *cfg, eeprom_access_t *access, uint16_t startAddress);
timesave_io_status_t restoreTime(timekeeper_t *tk, timesave_io_config_t *cfg);
timesave_io_status_t saveTime(timekeeper_t const *tk, timesave_io_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_TIMESAVE_IO_H */