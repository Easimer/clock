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
    uint8_t enabled;
} timer_state_t;

static timer_state_t timerStates[3];

void timerSetup(timer_id_t id) {
    if (id > TIMER_ID2) {
        return;
    }

    memset(&timerStates[id], 0, sizeof(timer_state_t));
}

void timerAddTimeElapsed(timer_id_t id, uint16_t millisElapsed) {
    timer_state_t *state = &timerStates[id];
    
    if(!state->enabled) {
        return;
    }

    for(uint8_t slotIdx = 0; slotIdx < TIMER_MAX_SUBSCRIBERS; slotIdx++) {
        timer_subscriber_t *subscriber = &state->subscribers[slotIdx];
        if(subscriber->used) {
            subscriber->callback(subscriber->user, millisElapsed);
        }
    }
}

timer_status_t timerSubscribe(timer_id_t id, timer_subscription_t *handle, void *user, timer_callback_t callback) {
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

void timerEnable(timer_id_t id) {
    if (id > TIMER_ID2) {
        return;
    }

    timerStates[id].enabled = 1;
}
