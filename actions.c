#include "actions.h"
#include "interrupts.h"
#include <string.h>

#define ACTIONS_MAX_BUTTONS (4)

#define INVOKE_CALLBACK_IF_NOT_NULL(descriptor, name, handle, user) \
    if(descriptor->name != NULL) { \
        descriptor->name(handle, user); \
    }

#define ACTIONS_DOUBLE_CLICK_MAX_MILLISECONDS_SINCE_LAST_RELEASE (250)
#define ACTIONS_LONG_PRESS_MIN_MILLISECONDS (250)

typedef struct actions_button {
    uint8_t used : 1;
    uint8_t held : 1;

    uint16_t millisHeld;
    uint16_t millisLastRelease;

    void *user;
    actions_button_descriptor_t *descriptor;
} actions_button_t;

static actions_button_t buttons[ACTIONS_MAX_BUTTONS];
static timer_subscription_t timerSub;

static void actionsTimerCallback(void *user, uint16_t millisElapsed) {
    for(uint8_t i = 0; i < ACTIONS_MAX_BUTTONS; i++) {
        actions_button_t *button = &buttons[i];
        if(button->used) {
            INVOKE_CALLBACK_IF_NOT_NULL(button->descriptor, probe, i, user);

            if(button->held) {
                button->millisHeld += millisElapsed;
            } else {
                button->millisLastRelease += millisElapsed;
            }
        }
    }
}

void actionsInit() {
    memset(buttons, 0, sizeof(buttons));
    timerSubscribe(TIMER_ID1, &timerSub, NULL, actionsTimerCallback);
}

actions_status_t actionsCreateButton(actions_button_handle_t *handle, void *user, actions_button_descriptor_t *descriptor) {
    if(handle == NULL || descriptor == NULL) {
        return EACTIONS_INVALID;
    }

    for(uint8_t i = 0; i < ACTIONS_MAX_BUTTONS; i++) {
        if(!buttons[i].used) {
            buttons[i].used = 1;
            buttons[i].held = 0;
            buttons[i].millisHeld = 0;
            buttons[i].user = user;
            buttons[i].descriptor = descriptor;
            return EACTIONS_OK;
        }
    }

    return EACTIONS_OUT_OF_MEMORY;
}

actions_status_t actionsDestroyButton(actions_button_handle_t handle) {
    if(handle >= ACTIONS_MAX_BUTTONS) {
        return EACTIONS_INVALID;
    }

    buttons[handle].used = 0;
    return EACTIONS_OK;
}

actions_status_t actionsSetButtonState(actions_button_handle_t handle, uint8_t isHeld) {
    if(handle >= ACTIONS_MAX_BUTTONS) {
        return EACTIONS_INVALID;
    }

    if(!buttons[handle].used) {
        return EACTIONS_INVALID;
    }

    actions_button_t *btn = &buttons[handle];

    if(!btn->held && isHeld) {
        btn->millisHeld = 0;
        
        if(btn->millisLastRelease < ACTIONS_DOUBLE_CLICK_MAX_MILLISECONDS_SINCE_LAST_RELEASE) {
            INVOKE_CALLBACK_IF_NOT_NULL(btn->descriptor, doubleClick, handle, btn->user);
            btn->millisLastRelease = 0;
        } else {
            INVOKE_CALLBACK_IF_NOT_NULL(btn->descriptor, pressed, handle, btn->user);
        }
    } else if(btn->held && !isHeld) {
        btn->millisLastRelease = 0;
        INVOKE_CALLBACK_IF_NOT_NULL(btn->descriptor, released, handle, btn->user);

        if(btn->millisHeld >= ACTIONS_LONG_PRESS_MIN_MILLISECONDS) {
            INVOKE_CALLBACK_IF_NOT_NULL(btn->descriptor, longPress, handle, btn->user);
        } else {
            INVOKE_CALLBACK_IF_NOT_NULL(btn->descriptor, click, handle, btn->user);
        }

        btn->millisHeld = 0;
    }

    btn->held = (isHeld != 0) ? 1 : 0;

    return EACTIONS_OK;
}