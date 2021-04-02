#include <assert.h>
#include "timekeeper.h"

// Time was set using timekeeperSet
#define TIMEKEEPER_FLAG_WASSET (1 << 0)

struct timekeeper {
    uint8_t flags;

    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint16_t millis;
};

void timekeeperInit(timekeeper_t *buffer) {
    buffer->flags = 0;
    buffer->hours = 0;
    buffer->minutes = 0;
    buffer->seconds = 0;
    buffer->millis = 0;
}

void timekeeperSet(timekeeper_t *tk, uint8_t hour, uint8_t minute) {
    while(minute > 60) {
        hour += 1;
        minute -= 60;
    }

    while(hour > 24) {
        hour -= 24;
    }

    tk->hours = hour;
    tk->minutes = minute;
    tk->seconds = 0;
    tk->millis = 0;
    tk->flags |= TIMEKEEPER_FLAG_WASSET;
}

void timekeeperGet(timekeeper_t const *tk, uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
    if(hours != NULL) *hours = tk->hours;
    if(minutes != NULL) *minutes = tk->minutes;
    if(seconds != NULL) *seconds = tk->seconds;
}

int timekeeperAccumulate(timekeeper_t *tk, uint16_t milliseconds) {
    int ret = 0;

    if(tk->flags & TIMEKEEPER_FLAG_WASSET) {
        ret = 1;
        tk->flags &= ~TIMEKEEPER_FLAG_WASSET;
    }

    // Avoid overflow due to addition
    while(milliseconds >= 1000) {
        tk->seconds += 1;
        milliseconds -= 1000;
        ret = 1;
    }

    tk->millis += milliseconds;

    while(tk->millis >= 1000) {
        tk->millis -= 1000;
        tk->seconds += 1;
        ret = 1;
    }

    assert(tk->millis < 1000);

    while(tk->seconds >= 60) {
        tk->seconds -= 60;
        tk->minutes += 1;
    }

    assert(tk->seconds < 60);

    while(tk->minutes >= 60) {
        tk->minutes -= 60;
        tk->hours += 1;
    }

    assert(tk->minutes < 60);

    while(tk->hours >= 24) {
        tk->hours -= 24;
    }

    assert(tk->hours < 60);

    return ret;
}