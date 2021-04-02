#include "config.h"
#include <Arduino.h>
#include "display.h"
#include "timekeeper.h"
#include "interrupts.h"
#include "actions.h"

typedef struct display_pins {
  uint8_t digit[4];
  uint8_t segment[7];
} display_pins_t;

static struct display_pins displayPins = {
  .digit = {
    PIN_DISPLAY_DIGIT0, PIN_DISPLAY_DIGIT1,
    PIN_DISPLAY_DIGIT2, PIN_DISPLAY_DIGIT3
  },
  .segment = {
    PIN_DISPLAY_SEGMENT_A, PIN_DISPLAY_SEGMENT_B, PIN_DISPLAY_SEGMENT_C, 
    PIN_DISPLAY_SEGMENT_D, PIN_DISPLAY_SEGMENT_E, PIN_DISPLAY_SEGMENT_F, 
    PIN_DISPLAY_SEGMENT_G
  },
};

static void display_select_cb(void *context, uint8_t digit) {
  display_pins_t *pins = (display_pins_t*)context;

  for(uint8_t i = 0; i < 4; i++) {
    digitalWrite(pins->digit[i], HIGH);
  }

  digitalWrite(pins->digit[digit], LOW);
}

static void display_segment_cb(void *context, uint8_t segment, uint8_t state) {
  display_pins_t *pins = (display_pins_t*)context;

  digitalWrite(pins->segment[segment], state != 0 ? HIGH : LOW);
}

static display_ctl_t displayCtl = {
  .context = &displayPins,
  .select = display_select_cb,
  .segment = display_segment_cb,
  
  .seq = 0,
};

static uint8_t digits[4];
static uint16_t then = 0;
TIMEKEEPER_DECLARE_BUFFER(tkTime);
static timer_subscription_t subscriptionDisplayDigits;

static void probeButton(actions_button_handle_t handle, void *user);
static void clickButton(actions_button_handle_t handle, void *user);
static void longPress(actions_button_handle_t handle, void *user);

static actions_button_handle_t btnIncreaseMinutes;
static actions_button_descriptor_t btnIncreaseMinutesDescriptor = {
  .probe = probeButton,
  .pressed = NULL,
  .released = NULL,
  .click = clickButton,
  .longPress = longPress,
  .doubleClick = NULL,
};

static void probeButton(actions_button_handle_t handle, void *user) {
  actionsSetButtonState(btnIncreaseMinutes, (digitalRead(PIN_BUTTON_3) == HIGH) ? 1 : 0);
}

static void clickButton(actions_button_handle_t handle, void *user) {
  if(handle == btnIncreaseMinutes) {
    uint8_t hours, minutes, seconds;
    timekeeperGet(tkTime, &hours, &minutes, &seconds);
    minutes++; // timekeeperSet will handle any overflow
    timekeeperSet(tkTime, hours, minutes);
  }
}

static void longPress(actions_button_handle_t handle, void *user) {
  if(handle == btnIncreaseMinutes) {
    uint8_t hours, minutes, seconds;
    timekeeperGet(tkTime, &hours, &minutes, &seconds);
    hours++; // timekeeperSet will handle any overflow
    timekeeperSet(tkTime, hours, minutes);
  }
}

static void displayDigits(void *user, uint16_t millis_elapsed) {
  displayDigitsDec(&displayCtl, digits[0], digits[1], digits[2], digits[3]);
}

void setup() {
  uint8_t rc;
  Serial.begin(9600);
  Serial.println("Init");

  for(uint8_t i = 0; i < 4; i++) {
    pinMode(displayPins.digit[i], OUTPUT);
    digitalWrite(displayPins.digit[i], HIGH);
  }

  for(uint8_t i = 0; i < 7; i++) {
    pinMode(displayPins.segment[i], OUTPUT);
    digitalWrite(displayPins.segment[i], LOW);
  }

  pinMode(PIN_BUTTON_3, INPUT);

  digits[0] = digits[1] = digits[2] = digits[3] = 0;

  cli();
  timerSetup(TIMER_ID1);
  sei();

  timekeeperInit(tkTime);

  actionsInit();
  if((rc = actionsCreateButton(&btnIncreaseMinutes, NULL, &btnIncreaseMinutesDescriptor)) != EACTIONS_OK) {
    Serial.print("actionsCreateButton failed: ");
    Serial.println(rc);
  }

  timerSubscribe(TIMER_ID1, &subscriptionDisplayDigits, NULL, displayDigits);
}

static void decomposeDigits(uint8_t num, uint8_t *dh, uint8_t *dl) {
  *dh = num / 10;
  *dl = num % 10;
}

void loop() {
  uint16_t now = millis();
  uint16_t elapsed = now - then;
  then = now;

  if(timekeeperAccumulate(tkTime, elapsed)) {
    uint8_t hours, minutes, seconds;
    timekeeperGet(tkTime, &hours, &minutes, &seconds);

    decomposeDigits(hours, &digits[0], &digits[1]);
    decomposeDigits(minutes, &digits[2], &digits[3]);
  }
}

#include "display.c"
#include "timekeeper.c"
#include "actions.c"