#ifndef CLOCK_ACTIONS_H
#define CLOCK_ACTIONS_H

#include <stdint.h>

typedef enum actions_status {
    EACTIONS_OK = 0,
    EACTIONS_INVALID,
    EACTIONS_OUT_OF_MEMORY,
    EACTIONS_NO_SUCH_BUTTON,
} actions_status_t;

typedef uint8_t actions_button_handle_t;

typedef void (*actions_button_probe_t)(actions_button_handle_t handle, void *user);
typedef void (*actions_button_pressed_t)(actions_button_handle_t handle, void *user);
typedef void (*actions_button_released_t)(actions_button_handle_t handle, void *user);
typedef void (*actions_button_clicked_t)(actions_button_handle_t handle, void *user);
typedef void (*actions_button_long_press_t)(actions_button_handle_t handle, void *user);
typedef void (*actions_button_double_clicked_t)(actions_button_handle_t handle, void *user);

typedef struct actions_button_descriptor {
    actions_button_probe_t probe;

    actions_button_pressed_t pressed;
    actions_button_pressed_t released;
    actions_button_clicked_t click;
    actions_button_long_press_t longPress;
    actions_button_double_clicked_t doubleClick;
} actions_button_descriptor_t;

#ifdef __cplusplus
extern "C" {
#endif

void actionsInit();
actions_status_t actionsCreateButton(actions_button_handle_t *handle, void *user, actions_button_descriptor_t *descriptor);
actions_status_t actionsDestroyButton(actions_button_handle_t handle);
actions_status_t actionsSetButtonState(actions_button_handle_t handle, uint8_t isHeld);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_ACTIONS_H */