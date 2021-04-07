#include "config.h"
#include <Arduino.h>
#include "core.h"
#include "log.h"

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

static uint8_t buttonProbe(uint8_t idx) {
  static uint8_t const pins[4] = { PIN_BUTTON_0, PIN_BUTTON_1, PIN_BUTTON_2, PIN_BUTTON_3 };

  if(idx > 3) {
    return 0;
  }

  return (digitalRead(pins[idx]) == LOW) ? 1 : 0;
}

static core_state_t coreState = {
  .displayCtl = &displayCtl,
  .buttonProbe = buttonProbe,
  .externalMemory = NULL,
};

static void setupTimer1Interrupts() {
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
}

ISR(TIMER1_COMPA_vect) {
    coreElapsed(&coreState, 1);
}

void setup() {
  uint8_t rc;
  Serial.begin(9600);

  l_str_ln("[+] Initializing display");

  for(uint8_t i = 0; i < 4; i++) {
    pinMode(displayPins.digit[i], OUTPUT);
    digitalWrite(displayPins.digit[i], HIGH);
  }

  for(uint8_t i = 0; i < 7; i++) {
    pinMode(displayPins.segment[i], OUTPUT);
    digitalWrite(displayPins.segment[i], LOW);
  }

  displayDigitsDec(&displayCtl, 8, 0, 0, 0);

  l_str_ln("[+] Initializing EEPROM");
  extmemInit();
  coreState.externalMemory = extmemGetAccess();

  l_str_ln("[+] Initializing buttons");
  pinMode(PIN_BUTTON_0, INPUT_PULLUP);
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);
  pinMode(PIN_BUTTON_3, INPUT_PULLUP);

  l_str_ln("[+] Setting up TIMER1 interrupts");
  cli();
  setupTimer1Interrupts();
  sei();

  l_str_ln("[+] Initializing core");
  coreInit(&coreState);

  l_str_ln("[+] Initialization done");
}

void loop() {
  coreLoop(&coreState);
}

#include "display.c"
#include "timekeeper.c"
#include "actions.c"
#include "eeprom_he.c"