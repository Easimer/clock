#include <stddef.h>
#include <stdint.h>
#include "7seg_bcd_mux.h"

// DP, G, F, E, D, C, B, A
static uint8_t digitSegmentBits[16] = {
    0b00111111,
    0b00000110,
    0b01011011,
    0b01001111,
    0b01100110,
    0b01101101,
    0b01111101,
    0b00000111,
    0b01111111,
    0b01101111,

    0b01110111,
    0b01111111,
    0b00111001,
    0b00111111,
    0b01111001,
    0b01110001,
};

void d7segDisplayDec(d7seg_ctl_t *ctl, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
    if(ctl == NULL) {
        return;
    }

    uint8_t const digits[4] = { d0, d1, d2, d3 };

    uint8_t idx = digits[ctl->seq] & 0xF;
    uint8_t bits = digitSegmentBits[idx];
    ctl->select(ctl->context, ctl->seq);

    for(uint8_t j = 0; j < 7; j++) {
        uint8_t state = bits & 1;
        ctl->segment(ctl->context, j, state);
        bits >>= 1;
    }

    ctl->seq = (ctl->seq + 1) & 3;
}
