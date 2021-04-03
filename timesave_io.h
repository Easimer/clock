#ifndef CLOCK_TIMESAVE_IO_H
#define CLOCK_TIMESAVE_IO_H

#include "eeprom_access.h"
#include "timekeeper.h"

typedef enum timesave_io_status {
    ETIMESAVE_IO_OK = 0,
    ETIMESAVE_IO_BADSIG,
    ETIMESAVE_IO_BADCHK,
    ETIMESAVE_IO_WRITE_FAILURE,
} timesave_io_status_t;

#ifdef __cplusplus
extern "C" {
#endif

timesave_io_status_t restoreTime(timekeeper_t *tk, eeprom_access_t *mem);
timesave_io_status_t saveTime(timekeeper_t const *tk, eeprom_access_t *mem);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_TIMESAVE_IO_H */