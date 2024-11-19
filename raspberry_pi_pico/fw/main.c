/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

extern void loop();

void usleep(uint32_t t) {
    sleep_us(t);
}

#define START_OF_FRAME_SIZE 4
#define END_OF_FRAME_SIZE   5
#define NUMBER_OF_LEDS      25
#define UPDATE_SIZE         (START_OF_FRAME_SIZE + END_OF_FRAME_SIZE + (4 * NUMBER_OF_LEDS))

static uint8_t update[UPDATE_SIZE];

void hw_set_rgb(uint8_t channel, uint8_t r, uint8_t g, uint8_t b) {
    if (channel < NUMBER_OF_LEDS) {
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
