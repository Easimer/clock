#include <assert.h>
#include "timekeeper.h"

struct timekeeper {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint16_t millis;
};

void timekeeper_init(timekeeper_t *buffer) {
    buffer->hours = 0;
    buffer->minutes = 0;
    buffer->seconds = 0;
    buffer->millis = 0;
}

void timekeeper_set(timekeeper_t *tk, uint8_t hour, uint8_t minute) {
    tk->hours = hour;
    tk->minutes = minute;
    tk->seconds = 0;
    tk->millis = 0;
}

void timekeeper_get(timekeeper_t const *tk, uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
    *hours = tk->hours;
    *minutes = tk->minutes;
    *seconds = tk->seconds;
}

int timekeeper_accumulate(timekeeper_t *tk, uint16_t milliseconds) {
    int ret = 0;

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