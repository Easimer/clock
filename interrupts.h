#ifndef CLOCK_INTERRUPTS_H
#define CLOCK_INTERRUPTS_H

#include <stdint.h>

typedef uint8_t timer_subscription_t;
typedef void (*timer_callback_t)(void *user, uint16_t millis_elapsed);

typedef enum timer_id {
    TIMER_ID0 = 0,
    TIMER_ID1,
    TIMER_ID2
} timer_id_t;

typedef enum timer_status {
    ETIMER_OK = 0,
    ETIMER_INVAL,
    ETIMER_MAX_SUBSCRIBERS,
    ETIMER_UNSUPPORTED,
} timer_status_t;

#ifdef __cplusplus
extern "C" {
#endif
    void timerSetup(timer_id_t id);
    void timerAddTimeElapsed(timer_id_t id, uint16_t millisElapsed);
    timer_status_t timerSubscribe(timer_id_t id, timer_subscription_t *handle, void *user, timer_callback_t callback);
    void timerUnsubscribe(timer_id_t id, timer_subscription_t handle);
#ifdef __cplusplus
}
#endif

#endif /* CLOCK_INTERRUPTS_H */