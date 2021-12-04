/*
 * program.c
 *
 * Copyright (c) 2014 Jeremy Garff <jer @ jers.net>
 * Copyright (c) 2021 Jack Whitham <jack.d.whitham@gmail.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     1.  Redistributions of source code must retain the above copyright notice, this list of
 *         conditions and the following disclaimer.
 *     2.  Redistributions in binary form must reproduce the above copyright notice, this list
 *         of conditions and the following disclaimer in the documentation and/or other materials
 *         provided with the distribution.
 *     3.  Neither the name of the owner nor the names of its contributors may be used to endorse
 *         or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdarg.h>
#include <getopt.h>
#include <sys/mount.h>


#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"
#include "version.h"

#include "ws2811.h"
#include "cpelight.h"

static char name_text[] = "made by Jack Whitham using lots of borrowed code! this version from 2/12/21";

#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))

// defaults for cmdline options
#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                18
#define DMA                     10
//#define STRIP_TYPE            WS2811_STRIP_RGB    // WS2812/SK6812RGB integrated chip+leds
#define STRIP_TYPE              WS2811_STRIP_GBR    // WS2812/SK6812RGB integrated chip+leds
//#define STRIP_TYPE            SK6812_STRIP_RGBW   // SK6812RGBW (NOT SK6812RGB)

#define WIDTH                   24
#define HEIGHT                  1
#define LED_COUNT               (WIDTH * HEIGHT)


ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .count = LED_COUNT,
            .invert = 0,
            .brightness = 255,
            .strip_type = STRIP_TYPE,
        },
        [1] =
        {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
            .brightness = 0,
        },
    },
};


void hw_show()
{
    ws2811_render(&ledstring);
}

void hw_set_rgb(uint8_t channel, uint8_t r, uint8_t g, uint8_t b)
{
    ledstring.channel[0].leds[channel] = 
        (((uint32_t) b) << 16) |
        (((uint32_t) g) << 8) |
        (((uint32_t) r) << 0);
}

extern void loop();

int main(int argc, char *argv[])
{
    ws2811_return_t ret;
    FILE * fd;

    printf("\n\nprogram.elf started\n%s\n", name_text);

    if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) != 0) {
        /* This failure is normal - the kernel mounts /dev */
    }
    if (mount("proc", "/proc", "proc", 0, NULL) != 0) {
        perror("mount proc /proc failed");
    }
    if (mount("tmpfs", "/tmp", "tmpfs", 0, NULL) != 0) {
        perror("mount tmpfs /tmp failed");
    }

    fd = fopen("/program.cfg", "rt");
    if (!fd) {
        printf("no configuration file - using default mode %d\n", colour_mode);
    } else {
        int i = 0;
        if (fscanf(fd, "%d", &i) == 1) {
            colour_mode = (t_colour_mode) (i % num_colour_modes);
            printf("colour mode set to %d\n", colour_mode);
        }
        fclose(fd);
    }

    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return 1;
    }

    printf("initialised hardware\n");
    fflush(stdout);
    loop();
    return 1;
}
