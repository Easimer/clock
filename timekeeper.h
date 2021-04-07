#ifndef CLOCK_TIMEKEEPER_H
#define CLOCK_TIMEKEEPER_H

#include <stdint.h>

#define TIMEKEEPER_BUFFER_SIZE (16)

typedef struct timekeeper timekeeper_t;

/*
 * Creates a buffer for a new timekeeper called `name`
 * 
 * @param name Name of the timekeeper_t variable.
 */
#define TIMEKEEPER_DECLARE_BUFFER(name) \
    uint8_t tkbuf_##name[TIMEKEEPER_BUFFER_SIZE]; \
    timekeeper_t *name = (timekeeper_t*)tkbuf_##name;

/*
 * Creates a new timekeeper in-place at the supplied pointer.
 * The space allocated at the pointer must be at least
 * TIMEKEEPER_BUFFER_SIZE long.
 * 
 * @param buffer Pointer to a buffer of size TIMEKEEPER_BUFFER_SIZE
 */
void timekeeperInit(timekeeper_t *buffer);

/*
 * Sets the state of the timekeeper. This will also reset the
 * seconds and the (internal) milliseconds counter.
 * 
 * This function will handle values of `minute` greater than 59 by
 * carrying over the extra minutes into the hour counter.
 * The same thing happens when `hour` is greater than 23.
 * 
 * @param tk Pointer to the timekeeper
 * @param hour New value of the hour counter
 * @param minute New value of the minute counter
 * @param second New value of the second counter
 */
void timekeeperSet(timekeeper_t *tk, uint8_t hour, uint8_t minute, uint8_t second);

/*
 * Reads the hour, minute and second counters from the timekeeper
 * and returns them
 * 
 * @param tk Pointer to the timekeeper
 * @param hours Pointer to the memory where the value of the hour
 * counter will be stored (may be NULL)
 * @param minutes Pointer to the memory where the value of the minute
 * counter will be stored (may be NULL)
 * @param seconds Pointer to the memory where the value of the second
 * counter will be stored (may be NULL)
 */
void timekeeperGet(timekeeper_t const *tk, uint8_t *hours, uint8_t *minutes, uint8_t *seconds);

/*
 * Adds a given number of milliseconds to a timekeeper.
 * 
 * @param tk Pointer to the timekeeper
 * @param milliseconds Number of milliseconds to add
 * @return A value indicating whether the state of the timekeeper
 * changed since the last call to timekeeperAccumulate.
 * This value is
 * - 0 if there was no change;
 * - 1 if only the `seconds` counter has changed,
 * - 2 if also the `minutes` counter has changed and
 * - 3 if the `hours` counter has changed as well.
 */
int timekeeperAccumulate(timekeeper_t *tk, uint16_t milliseconds);

#endif /* CLOCK_TIMEKEEPER_H */