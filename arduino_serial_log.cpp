#include "log.h"
#include <Arduino.h>

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