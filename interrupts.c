#include <Arduino.h>
#include <string.h>
#include "interrupts.h"

#define TIMER_MAX_SUBSCRIBERS (4)

typedef struct timer_subscriber {
    uint8_t used;
    void *user;
    timer_callback_t callback;
} timer_subscriber_t;

typedef struct timer_state {
    timer_subscriber_t subscribers[TIMER_MAX_SUBSCRIBERS];
} timer_state_t;

static timer_state_t timerStates[3];

void timer_setup(timer_id_t id) {
    if (id > TIMER_ID2) {
        return;
    }

    memset(&timerStates[id], 0, sizeof(timer_state_t));

    switch(id) {
        case TIMER_ID1:
            // Timer will interrupt every millisecond
            // Clear timer control registers
            TCCR1A = TCCR1B = 0;
            // Clear timer counter
            TCNT1 = 0;
            // Set waveform generation mode to mode 12 (Clear Timer on Compare)
            TCCR1B |= (1 << WGM12);
            // Set Compare Match Register to 63
            OCR1A = 249;
            // Set clock select bits in the Timer Control Register B to 0b011
            // which means the prescaler will be 64.
            TCCR1B |= (1 << CS11) | (1 << CS10);
            // Enable timer compare interrupt
            TIMSK1 |= (1 << OCIE1A);
            break;
    }
}

timer_status_t timer_subscribe(timer_id_t id, timer_subscription_t *handle, void *user, timer_callback_t callback) {
    if(id > TIMER_ID2 || handle == NULL || callback == NULL) {
        return ETIMER_INVAL;
    }

    if(id == TIMER_ID0 || id == TIMER_ID2) {
        return ETIMER_UNSUPPORTED;
    }

    timer_state_t *state = &timerStates[id];

    for(uint8_t slotIdx = 0; slotIdx < TIMER_MAX_SUBSCRIBERS; slotIdx++) {
        if(!state->subscribers[slotIdx].used) {
            state->subscribers[slotIdx].used = 1;
            state->subscribers[slotIdx].user = user;
            state->subscribers[slotIdx].callback = callback;
            *handle = slotIdx;
            return ETIMER_OK;
        }
    }

    return ETIMER_MAX_SUBSCRIBERS;
}

void timer_unsubscribe(timer_id_t id, timer_subscription_t handle);

ISR(TIMER1_COMPA_vect) {
    timer_state_t *state = &timerStates[TIMER_ID1];
    for(uint8_t slotIdx = 0; slotIdx < TIMER_MAX_SUBSCRIBERS; slotIdx++) {
        timer_subscriber_t *subscriber = &state->subscribers[slotIdx];
        if(subscriber->used) {
            subscriber->callback(subscriber->user, 1);
        }
    }
}
