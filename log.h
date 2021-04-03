#ifndef CLOCK_LOG_H
#define CLOCK_LOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void logPrintLnString(char const *s);
void logPrintString(char const *s);

void logPrintLnNumber(uint16_t);
void logPrintNumber(uint16_t);

#ifdef __cplusplus
}
#endif

#endif