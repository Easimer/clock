#include "config.h"
#include <Arduino.h>
#include "display.h"
#include "timekeeper.h"
#include "interrupts.h"

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

static void displayDigits(void *user, uint16_t millis_elapsed) {
  display_digits_dec(&displayCtl, digits[0], digits[1], digits[2], digits[3]);
}

void setup() {
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

  digits[0] = digits[1] = digits[2] = digits[3] = 0;

  timekeeper_init(tkTime);

  cli();

  timer_setup(TIMER_ID1);
  timer_subscribe(TIMER_ID1, &subscriptionDisplayDigits, NULL, displayDigits);

  sei();
}

static void decomposeDigits(uint8_t num, uint8_t *dh, uint8_t *dl) {
  *dh = num / 10;
  *dl = num % 10;
}

void loop() {
  uint16_t now = millis();
  uint16_t elapsed = now - then;
  then = now;

  if(timekeeper_accumulate(tkTime, elapsed)) {
    uint8_t hours, minutes, seconds;
    timekeeper_get(tkTime, &hours, &minutes, &seconds);

    decomposeDigits(hours, &digits[0], &digits[1]);
    decomposeDigits(minutes, &digits[2], &digits[3]);
  }
}

#include "display.c"
#include "timekeeper.c"