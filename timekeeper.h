#ifndef CLOCK_TIMEKEEPER_H
#define CLOCK_TIMEKEEPER_H

#include <stdint.h>

#define TIMEKEEPER_BUFFER_SIZE (16)

typedef struct timekeeper timekeeper_t;

#define TIMEKEEPER_DECLARE_BUFFER(name) \
    uint8_t tkbuf_##name[TIMEKEEPER_BUFFER_SIZE]; \
    timekeeper_t *name = (timekeeper_t*)tkbuf_##name;

void timekeeper_init(timekeeper_t *buffer);
void timekeeper_set(timekeeper_t *tk, uint8_t hour, uint8_t minute);
void timekeeper_get(timekeeper_t const *tk, uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
int timekeeper_accumulate(timekeeper_t *tk, uint16_t milliseconds);

#endif /* CLOCK_TIMEKEEPER_H */