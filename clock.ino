#include "config.h"
#include "core.h"
#include "kprintf.h"
#include "display_7seg.h"

#if DISPLAY_USE_7SEG

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

static d7seg_ctl_t display7SegCtl = {
  .context = &displayPins,
  .select = display_select_cb,
  .segment = display_segment_cb,
  
  .seq = 0,
};

static display_7seg_t display;

#endif /* DISPLAY_USE_7SEG */

static uint8_t buttonProbe(uint8_t idx) {
  static uint8_t const pins[4] = { PIN_BUTTON_0, PIN_BUTTON_1, PIN_BUTTON_2, PIN_BUTTON_3 };

  if(idx > 3) {
    return 0;
  }

  return (digitalRead(pins[idx]) == LOW) ? 1 : 0;
}

static core_state_t coreState = {
  .displayHw = &display.display,
  .displayHwUser = &display,

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

extern "C" {

void logPrintLnString(char const *s) {
    Serial.println(s);
}

void logPrintString(char const *s) {
    Serial.print(s);
}

void logPrintLnNumber(uint16_t n) {
    Serial.println(n);
}

void logPrintNumber(uint16_t n) {
    Serial.print(n);
}

}

ISR(TIMER1_COMPA_vect) {
    coreElapsed(&coreState, 1);
}

void setup() {
  Serial.begin(115200);

  kprintf(LOG_SUCCESS "Initializing display\n");

  for(uint8_t i = 0; i < 4; i++) {
    pinMode(displayPins.digit[i], OUTPUT);
    digitalWrite(displayPins.digit[i], HIGH);
  }

  for(uint8_t i = 0; i < 7; i++) {
    pinMode(displayPins.segment[i], OUTPUT);
    digitalWrite(displayPins.segment[i], LOW);
  }

#if DISPLAY_USE_7SEG
  d7segDisplayDec(&display7SegCtl, 8, 0, 0, 0);
  d7segInitDescriptor(&display, &display7SegCtl);
#endif /* DISPLAY_USE_7SEG */

  kprintf(LOG_SUCCESS "Initializing EEPROM\n");
  extmemInit();
  coreState.externalMemory = extmemGetAccess();

  kprintf(LOG_SUCCESS "Initializing buttons\n");
  pinMode(PIN_BUTTON_0, INPUT_PULLUP);
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);
  pinMode(PIN_BUTTON_3, INPUT_PULLUP);

  kprintf(LOG_SUCCESS "Setting up TIMER1 interrupts\n");
  cli();
  setupTimer1Interrupts();
  sei();

  kprintf(LOG_SUCCESS "Initializing core\n");
  coreInit(&coreState);

  kprintf(LOG_SUCCESS "Initialization done\n");

  timerEnable(TIMER_ID1);
}

void loop() {
  coreLoop(&coreState);
}
