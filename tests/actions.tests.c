#include "utest.h"

#include "config.h"
#include "actions.h"

void testPress(actions_button_handle_t handle, void *user);
void testRelease(actions_button_handle_t handle, void *user);
void testClick(actions_button_handle_t handle, void *user);
void testLongPress(actions_button_handle_t handle, void *user);
void testDoubleClick(actions_button_handle_t handle, void *user);

static actions_button_descriptor_t const testDescriptor = {
    .probe = NULL,

    .pressed = testPress,
    .released = testRelease,
    .click = testClick,
    .longPress = testLongPress,
    .doubleClick = testDoubleClick 
};

typedef struct Actions {
    actions_button_handle_t button;

    int pressed;
    int released;
    int click;
    int longPress;
    int doubleClick;
} actions_state_t;

UTEST_F_SETUP(Actions) {
    actions_status_t rc;
    
    utest_fixture->pressed = 0;
    utest_fixture->released = 0;
    utest_fixture->click = 0;
    utest_fixture->longPress = 0;
    utest_fixture->doubleClick = 0;

    actionsInit();
    rc = actionsCreateButton(&utest_fixture->button, utest_fixture, &testDescriptor);
    ASSERT_EQ(rc, EACTIONS_OK);
}

UTEST_F_TEARDOWN(Actions) {
    actions_status_t rc;
    
    rc = actionsDestroyButton(utest_fixture->button);
    ASSERT_EQ(rc, EACTIONS_OK);
}

UTEST_F(Actions, CreateButton) {
    actions_button_handle_t handle;
    actions_status_t rc;
    actions_button_descriptor_t d = {
        .probe = NULL,

        .pressed = NULL,
        .released = NULL,
        .click = NULL,
        .longPress = NULL,
        .doubleClick = NULL
    };

    rc = actionsCreateButton(&handle, NULL, &d);
    ASSERT_EQ(rc, EACTIONS_OK);
    
    actionsDestroyButton(handle);
}

#define ASSERT_ACTIONS_STATE(state, expectedPressed, expectedReleased, expectedClick, expectedLongPress, expectedDoubleClick)   \
    ASSERT_EQ(state->pressed, expectedPressed);													                                \
    ASSERT_EQ(state->released, expectedReleased);												                                \
    ASSERT_EQ(state->click, expectedClick);														                                \
    ASSERT_EQ(state->longPress, expectedLongPress);												                                \
    ASSERT_EQ(state->doubleClick, expectedDoubleClick);

UTEST_F(Actions, Pressed) {
    actions_status_t rc;

    rc = actionsSetButtonState(utest_fixture->button, 1);
    ASSERT_EQ(rc, EACTIONS_OK);
    ASSERT_EQ(utest_fixture->pressed, 1);
}

UTEST_F(Actions, Released) {
    actions_status_t rc;

    rc = actionsSetButtonState(utest_fixture->button, 1);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsSetButtonState(utest_fixture->button, 0);
    ASSERT_EQ(rc, EACTIONS_OK);
    ASSERT_EQ(utest_fixture->released, 1);
}

UTEST_F(Actions, Click) {
    actions_status_t rc;

    rc = actionsSetButtonState(utest_fixture->button, 1);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsTimeElapsed(10);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsSetButtonState(utest_fixture->button, 0);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsTimeElapsed(ACTIONS_DOUBLE_CLICK_MAX_MILLISECONDS_SINCE_LAST_RELEASE + 1);
    ASSERT_EQ(rc, EACTIONS_OK);

    ASSERT_ACTIONS_STATE(utest_fixture, 1, 1, 1, 0, 0);
}

UTEST_F(Actions, LongPress) {
    actions_status_t rc;

    rc = actionsSetButtonState(utest_fixture->button, 1);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsTimeElapsed(ACTIONS_LONG_PRESS_MIN_MILLISECONDS);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsSetButtonState(utest_fixture->button, 0);
    ASSERT_EQ(rc, EACTIONS_OK);

    ASSERT_ACTIONS_STATE(utest_fixture, 1, 1, 0, 1, 0);
}

UTEST_F(Actions, DoubleClick) {
    actions_status_t rc;

    rc = actionsSetButtonState(utest_fixture->button, 1);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsTimeElapsed(10);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsSetButtonState(utest_fixture->button, 0);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsTimeElapsed(ACTIONS_DOUBLE_CLICK_MAX_MILLISECONDS_SINCE_LAST_RELEASE / 2);
    ASSERT_EQ(rc, EACTIONS_OK);

    rc = actionsSetButtonState(utest_fixture->button, 1);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsTimeElapsed(10);
    ASSERT_EQ(rc, EACTIONS_OK);
    rc = actionsSetButtonState(utest_fixture->button, 0);
    ASSERT_EQ(rc, EACTIONS_OK);

    ASSERT_ACTIONS_STATE(utest_fixture, 1, 1, 0, 0, 1);
}

void testPress(actions_button_handle_t handle, void *user) {
    actions_state_t *state = (actions_state_t *)user;
    state->pressed = 1;
}

void testRelease(actions_button_handle_t handle, void *user) {
    actions_state_t *state = (actions_state_t *)user;
    state->released = 1;
}

void testClick(actions_button_handle_t handle, void *user) {
    actions_state_t *state = (actions_state_t *)user;
    state->click = 1;
}

void testLongPress(actions_button_handle_t handle, void *user) {
    actions_state_t *state = (actions_state_t *)user;
    state->longPress = 1;
}

void testDoubleClick(actions_button_handle_t handle, void *user) {
    actions_state_t *state = (actions_state_t *)user;
    state->doubleClick = 1;
}
