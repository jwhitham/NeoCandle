/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

extern void loop();

void usleep(uint32_t t) {
    sleep_us(t);
}

#define START_OF_FRAME_SIZE 4
#define END_OF_FRAME_SIZE   5
#define UPDATE_SIZE         (START_OF_FRAME_SIZE + END_OF_FRAME_SIZE + (4 * NUMBER_OF_LEDS))

static uint8_t update[UPDATE_SIZE];
static uint8_t remap[NUMBER_OF_LEDS] = {
                    //  Pi Hut 3D Christmas tree (TPH-017)
                    //  LEDs are connected via SPI and numbered from 0
        19, 20, 21, //  1 O'Clock - 19 bottom, 20, 21 top
        7, 8, 9,    //  2 O'Clock - 7 bottom, 8, 9 top
        0, 1, 2,    //  4 O'clock - 0 bottom, 1, 2 top
        16, 17, 18, //  5 O'Clock - 16 bottom, 17, 18 top
        15, 14, 13, //  7 O'Clock - 13 top, 14, 15 bottom
        6, 5, 4,    //  8 O'Clock - 4 top, 5, 6 bottom
        12, 11, 10, //  10 O'Clock - 10 top, 11, 12 bottom
        24, 23, 22, //  11 O'Clock - 22 top, 23, 24 bottom
        3};         //  Top is 3


void hw_set_rgb(uint8_t channel, uint8_t r, uint8_t g, uint8_t b) {
    if (channel < NUMBER_OF_LEDS) {
        channel = remap[channel];
        update[START_OF_FRAME_SIZE + (4 * channel)] = 0xef;
        update[START_OF_FRAME_SIZE + (4 * channel) + 1] = b;
        update[START_OF_FRAME_SIZE + (4 * channel) + 2] = g;
        update[START_OF_FRAME_SIZE + (4 * channel) + 3] = r;
    }
}

void hw_show(void) {
    spi_write_blocking(spi_default, update, UPDATE_SIZE);
}


int main() {
    stdio_init_all();

#if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
    puts("Default SPI pins were not defined");
#else

    // This example will use SPI0 at 1MHz.
    spi_init(spi_default, 1 * 1000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    // Make the SPI pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);

    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

    loop();
#endif
    return 0;
}
