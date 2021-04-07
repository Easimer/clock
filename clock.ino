#include "config.h"
#include <Arduino.h>
#include "display.h"
#include "timekeeper.h"
#include "interrupts.h"
#include "actions.h"
#include "eeprom_access.h"
#include "timesave_io.h"

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
TIMEKEEPER_DECLARE_BUFFER(tkTime);
static timesave_io_config_t tsiocTime;
static timer_subscription_t subscriptionDisplayDigits;
static timer_subscription_t subscriptionAccumulateTime;

static void probeButton(actions_button_handle_t handle, void *user);
static void clickButton(actions_button_handle_t handle, void *user);
static void longPress(actions_button_handle_t handle, void *user);

static actions_button_handle_t btnStopwatch;
static actions_button_handle_t btnIncreaseMinutes;
static actions_button_descriptor_t btnCommonDescriptor = {
  .probe = probeButton,
  .pressed = NULL,
  .released = NULL,
  .click = clickButton,
  .longPress = longPress,
  .doubleClick = NULL,
};

static void probeButton(actions_button_handle_t handle, void *user) {
  uint8_t state = 0;

  if(handle == btnStopwatch) {
    state = (digitalRead(PIN_BUTTON_2) == LOW) ? 1 : 0;
  } else if(handle == btnIncreaseMinutes) {
    state = (digitalRead(PIN_BUTTON_3) == LOW) ? 1 : 0;
  }

  actionsSetButtonState(handle, state);
}

static void clickButton(actions_button_handle_t handle, void *user) {
  if(handle == btnIncreaseMinutes) {
    uint8_t hours, minutes, seconds;
    timekeeperGet(tkTime, &hours, &minutes, &seconds);
    minutes++; // timekeeperSet will handle any overflow
    timekeeperSet(tkTime, hours, minutes, seconds);
  }
}

static void longPress(actions_button_handle_t handle, void *user) {
  if(handle == btnIncreaseMinutes) {
    uint8_t hours, minutes, seconds;
    timekeeperGet(tkTime, &hours, &minutes, &seconds);
    hours++; // timekeeperSet will handle any overflow
    timekeeperSet(tkTime, hours, minutes, seconds);
  }
}

static void displayDigits(void *user, uint16_t millis_elapsed) {
  displayDigitsDec(&displayCtl, digits[0], digits[1], digits[2], digits[3]);
}

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

static void saveTimeToEEPROM(timekeeper_t const *tk) {
  timesave_io_status_t rc;
  rc = saveTime(tk, &tsiocTime);

  switch(rc) {
    case ETIMESAVE_IO_OK:
      Serial.println("[+] Saved time");
      break;
    case ETIMESAVE_IO_ERASED:
      Serial.println("[+] timesave: first initialization");
      break;
    case ETIMESAVE_IO_WRITE_FAILURE:
      Serial.println("[-] timesave: couldn't save time: write failure");
      break;
  }
}

static void restoreTimeFromEEPROM(timekeeper_t *tk) {
  timesave_io_status_t rc;

  Serial.println("[+] Restoring time from EEPROM...");

  rc = restoreTime(tk, &tsiocTime);

  switch(rc) {
    case ETIMESAVE_IO_OK:
      Serial.println("[+] Restored time");
      break;
    case ETIMESAVE_IO_BADSIG:
      Serial.println("[-] Couldn't restore time: bad signature (old version?)");
      break;
    case ETIMESAVE_IO_BADCHK:
      Serial.println("[-] Couldn't restore time: bad checksum");
      break;
    case ETIMESAVE_IO_READ_FAILURE:
      Serial.println("[-] Couldn't restore time: read failure");
      break;
    case ETIMESAVE_IO_WRITE_FAILURE:
      Serial.println("[-] Couldn't restore time: write failure");
      break;
    case ETIMESAVE_IO_ERASED:
      Serial.println("[+] Time wasn't restored: first initialization");
      break;
  }
}

static struct {
  uint8_t minutesElapsed;
} timesaveState = {
  .minutesElapsed = 0,
};
static uint8_t saveTimeToROM = 0;

static void accumulateTime(void *user, uint16_t millisElapsed) {
  int change;
  if((change = timekeeperAccumulate(tkTime, millisElapsed)) != 0) {
    uint8_t hours, minutes, seconds;
    timekeeperGet(tkTime, &hours, &minutes, &seconds);

    decomposeDigits(hours, &digits[0], &digits[1]);
    decomposeDigits(minutes, &digits[2], &digits[3]);

    if(change > 1 || change < -1) {
      timesaveState.minutesElapsed += 1;

      if(timesaveState.minutesElapsed >= 5) {
        saveTimeToROM = 1;
        timesaveState.minutesElapsed = 0;
      }
    }
  }
}

ISR(TIMER1_COMPA_vect) {
    timerAddTimeElapsed(TIMER_ID1, 1);
}

void setup() {
  uint8_t rc;
  Serial.begin(9600);

  Serial.println("[+] Initializing display");

  for(uint8_t i = 0; i < 4; i++) {
    pinMode(displayPins.digit[i], OUTPUT);
    digitalWrite(displayPins.digit[i], HIGH);
  }

  for(uint8_t i = 0; i < 7; i++) {
    pinMode(displayPins.segment[i], OUTPUT);
    digitalWrite(displayPins.segment[i], LOW);
  }

  displayDigitsDec(&displayCtl, 8, 0, 0, 0);

  Serial.println("[+] Initializing EEPROM");
  extmemInit();

  Serial.println("[+] Initializing buttons");
  pinMode(PIN_BUTTON_0, INPUT_PULLUP);
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);
  pinMode(PIN_BUTTON_3, INPUT_PULLUP);

  digits[0] = digits[1] = digits[2] = digits[3] = 0;

  Serial.println("[+] Initializing timers");
  cli();
  setupTimer1Interrupts();
  sei();

  timerSetup(TIMER_ID1);

  Serial.println("[+] Creating main timekeeper");
  timekeeperInit(tkTime);

  Serial.println("[+] Initializing time save/restore mechanism");
  timesave_io_status_t tsioStatus = fillTimesaveConfig(&tsiocTime, extmemGetAccess(), 0);
  if(tsioStatus != ETIMESAVE_IO_OK) {
    Serial.print("[+] timesave: fillTimesaveConfig failed with rc=");
    Serial.println(tsioStatus);
  } else {
    restoreTimeFromEEPROM(tkTime);
  }

  Serial.println("[+] Setting up actions subsystem");
  actionsInit();
  
  if((rc = actionsCreateButton(&btnIncreaseMinutes, NULL, &btnCommonDescriptor)) != EACTIONS_OK) {
    Serial.print("actionsCreateButton failed: ");
    Serial.println(rc);
  }

  if((rc = actionsCreateButton(&btnStopwatch, NULL, &btnCommonDescriptor)) != EACTIONS_OK) {
    Serial.print("actionsCreateButton failed: ");
    Serial.println(rc);
  }

  timerSubscribe(TIMER_ID1, &subscriptionDisplayDigits, NULL, displayDigits);
  timerSubscribe(TIMER_ID1, &subscriptionAccumulateTime, NULL, accumulateTime);

  Serial.println("[+] Init OK");
}

static void decomposeDigits(uint8_t num, uint8_t *dh, uint8_t *dl) {
  *dh = num / 10;
  *dl = num % 10;
}

void loop() {
  if(saveTimeToROM) {
    saveTimeToROM = 0;
    saveTimeToEEPROM(tkTime);
  }
}

#include "display.c"
#include "timekeeper.c"
#include "actions.c"
#include "eeprom_he.c"