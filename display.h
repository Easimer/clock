#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

typedef enum display_digit {
  DISP_DIGIT0 = 0,
  DISP_DIGIT1,
  DISP_DIGIT2,
  DISP_DIGIT3,
} display_digit_t;

typedef enum display_segment {
  DISP_SEG_A = 0,
  DISP_SEG_B,
  DISP_SEG_C,
  DISP_SEG_D,
  DISP_SEG_E,
  DISP_SEG_F,
  DISP_SEG_G,
} display_segment_t;

typedef void (*display_ctl_select)(void *context, uint8_t digit);
typedef void (*display_ctl_segment)(void *context, uint8_t segment, uint8_t state);

typedef struct display_ctl {
  void *context;
  display_ctl_select select;
  display_ctl_segment segment;
  uint8_t seq;
} display_ctl_t;

void display_digits_dec(display_ctl_t *ctl, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);

#endif /* CLOCK_DISPLAY_H */
