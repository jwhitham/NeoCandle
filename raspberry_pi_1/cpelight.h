#ifndef CPELIGHT_H
#define CPELIGHT_H

#include <stdint.h>

void hw_set_rgb(uint8_t channel, uint8_t r, uint8_t g, uint8_t b);
void hw_show(void);

typedef enum {
    candle = 0,
    red,
    green,
    blue,
    white,
    rainbow_1,
    rainbow_2,
    num_colour_modes} t_colour_mode;

extern t_colour_mode colour_mode;

#endif
