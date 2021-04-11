#ifndef CLOCK_7SEG_H
#define CLOCK_7SEG_H

/*
 * Hardware abstraction for 4 digit 7-segment displays
 */

typedef enum d7seg_digit {
  D7SEG_DIGIT0 = 0,
  D7SEG_DIGIT1,
  D7SEG_DIGIT2,
  D7SEG_DIGIT3,
} d7seg_digit_t;

typedef enum d7seg_segment {
  D7SEG_SEG_A = 0,
  D7SEG_SEG_B,
  D7SEG_SEG_C,
  D7SEG_SEG_D,
  D7SEG_SEG_E,
  D7SEG_SEG_F,
  D7SEG_SEG_G,
} d7seg_segment_t ;

typedef void (*d7seg_ctl_select)(void *context, uint8_t digit);
typedef void (*d7seg_ctl_segment)(void *context, uint8_t segment, uint8_t state);

typedef struct d7seg_ctl {
  void *context;
  d7seg_ctl_select select;
  d7seg_ctl_segment segment;
  uint8_t seq;
} d7seg_ctl_t;

#ifdef __cplusplus
extern "C" {
#endif

void d7segDisplayDec(d7seg_ctl_t *ctl, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_7SEG_H */
