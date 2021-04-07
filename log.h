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

#define l_str_ln(s) logPrintLnString(s)
#define l_str(s) logPrintString(s)

#define l_num_ln(s) logPrintLnNumber(s)
#define l_num(s) logPrintNumber(s)

#endif