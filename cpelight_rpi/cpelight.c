// candle for Raspberry Pi driving a ring of NeoPixel-like devices
// based on 8 pixel version by Tim Bartlett, December 2013
// Modified by Jack Whitham for Adafruit Circuit Playground Express board.
// Modified again to use the rainbow mode only depending on switch setting.
// Modified again for Raspberry Pi
//
//
#include <stdlib.h>
#include <setjmp.h>
#include <jack_coroutines.h>
#include <unistd.h>

#include "cpelight.h"

t_colour_mode colour_mode = rainbow_2;
static unsigned cycle_counter = 0;


// color variables: mix RGB (0-255) for desired yellow
static const int redPx = 255;
static const int grnHigh = 70;
static const int bluePx = 5;

static const float candleness = 1.5; // amount of flickeriness

// animation time variables, with recommendations
static const int burnDepth = 6 * candleness; //how much green dips below grnHigh for normal burn - 
static const int flutterDepth = 20 * candleness; //maximum dip for flutter
static const int cycleTime = 120; //duration of one dip in milliseconds

// pay no attention to that man behind the curtain
static const int flickerDepth = (burnDepth + flutterDepth) / 2.4;
static const int burnLow = grnHigh - burnDepth;
static const int burnDelay = (cycleTime / 2) / burnDepth;
static const int flickLow = grnHigh - flickerDepth;
static const int flickDelay = (cycleTime / 2) / flickerDepth;
static const int flutLow = grnHigh - flutterDepth;
static const int flutDelay = ((cycleTime / 2) / flutterDepth);

// In loop, call CANDLE STATES, with duration in seconds
// 1. on() = solid yellow
// 2. burn() = candle is burning normally, flickering slightly
// 3. flicker() = candle flickers noticably
// 4. flutter() = the candle needs air!

#define num_channels 24
#define task_stack_space 4096    // stack bytes per task

static jmp_buf task_table[num_channels + 1];
static long all_stack_space[(task_stack_space * num_channels) / sizeof(long)];

static int ch_constrain(int x, int l, int h)
{
    if (x < l) return l;
    if (x > h) return h;
    return x;
}

static void set_rainbow_rgb(int channel, int scale, int * r, int * g, int * b)
{
    // Adafruit rainbow colour wheel
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    unsigned char WheelPos =
        (((channel * 256) / ((unsigned) num_channels * scale)) + (cycle_counter / 64)) & 255;

    if(WheelPos < 85) {
        *r = 255 - WheelPos * 3;
        *g = 0;
        *b = WheelPos * 3;
    } else {
        if(WheelPos < 170) {
            WheelPos -= 85;
            *r = 0;
            *g = WheelPos * 3;
            *b = 255 - WheelPos * 3;
        } else {
            WheelPos -= 170;
            *r = WheelPos * 3;
            *g = 255 - WheelPos * 3;
            *b = 0;
        }
    }
}

static void set_rgb(int channel, int r, int g, int b, int mono)
{
    unsigned scale = 1;
    switch (colour_mode) {
        case red:
            r = mono;
            g = b = 0;
            break;
        case green:
            g = mono;
            r = b = 0;
            break;
        case blue:
            b = mono;
            g = r = 0;
            break;
        case white:
            r = g = b = mono;
            break;
        case rainbow_1:
            scale = 8;
        case rainbow_2:
            set_rainbow_rgb (channel, scale, &r, &g, &b);
            r = (r * mono) / 256;
            g = (g * mono) / 256;
            b = (b * mono) / 256;
            break;
        default:
            break;
    }

    r = ch_constrain(r, 0, 255);
    g = ch_constrain(g, 0, 255);
    b = ch_constrain(b, 0, 255);
    hw_set_rgb(channel, r, g, b);
}

static void yield(int channel, int fDelay)
{
    while (fDelay > 0) {
        fDelay --;
        coroutines_goto_next_task (task_table[channel], task_table[channel + 1]);
    }
}

// basic fire funciton - not called in main loop
static void fire(int channel, int grnLow, int fDelay)
{
    int grnPx;
    for (grnPx = grnHigh; grnPx > grnLow; grnPx--) {
        set_rgb(channel, redPx, grnPx, bluePx, (grnPx * redPx) / grnHigh); // 4:
        yield(channel, fDelay);
    }  
    for (grnPx = grnLow; grnPx < grnHigh; grnPx++) {
        set_rgb(channel, redPx, grnPx, bluePx, (grnPx * redPx) / grnHigh); // 4:
        yield(channel, fDelay);
    }
}

// fire animation
static void on(int channel, int f)
{
    const int grnPx = grnHigh - 6;
    const int fRep = f * 10;
    int i;
    for (i = 0; i < fRep; i++) {
        set_rgb(channel, redPx, grnPx, bluePx, redPx); // 4: 
        yield(channel, 100);
    }
}

static void burn(int channel, int f)
{
    const int fRep = f * 8;
    const int fDelay = burnDelay;
    for (int var = 0; var < fRep; var++) {
        fire(channel, burnLow, fDelay);
    }  
}

static void flicker(int channel, int f)
{
    const int fRep = f * 8;
    int fDelay = burnDelay;
    fire(channel, burnLow, fDelay);
    fDelay = flickDelay;
    for (int var = 0; var < fRep; var++) {
        fire(channel, flickLow, fDelay);
    }
    fDelay = burnDelay;
    fire(channel, burnLow, fDelay);
    fire(channel, burnLow, fDelay);
    fire(channel, burnLow, fDelay);
}

static void flutter(int channel, int f)
{
    const int fRep = f * 8;  
    int fDelay = burnDelay;
    fire(channel, burnLow, fDelay);
    fDelay = flickDelay;
    fire(channel, flickLow, fDelay);
    fDelay = flutDelay;
    for (int var = 0; var < fRep; var++) {
        fire(channel, flutLow, fDelay);
    }
    fDelay = flickDelay;
    fire(channel, flickLow, fDelay);
    fire(channel, flickLow, fDelay);
    fDelay = burnDelay;
    fire(channel, burnLow, fDelay);
    fire(channel, burnLow, fDelay);
}

static int approx (int midpoint)
{
    return ch_constrain (midpoint + (rand () % 5) - 2, 1, 20);
}

static void task_0_to_9 (jmp_buf ignore, void * arg)
{
    const int channel = (int) arg;
    int state = channel;
    (void) ignore;
    set_rgb(channel, 0, 0, 0, 0);
    yield(channel, 1 + (channel * 100));
    while (1) {
        switch (state) {
            case 0:
                burn(channel, approx(10));
            case 1:
                flicker(channel, approx(5));
            case 2:
                burn(channel, approx(8));
            case 3:
                flutter(channel, approx(6));
            case 4:
                burn(channel, approx(3));
            case 5:
                on(channel, approx(1));
            case 6:
                burn(channel, approx(10));
            case 7:
                flicker(channel, approx(10));
            case 8:
                on(channel, approx(1));
            default:
                flutter(channel, approx(10));
        }
        state = 0;
    }
}

void loop ()
{
    int channel;

    for (channel = 0; channel < num_channels; channel++) {
        coroutines_setup_task
            (task_table[channel + 1],
             task_table[channel],
             &all_stack_space[((channel + 1) * task_stack_space) / sizeof (long)],
             task_0_to_9, (void *) channel);
    }

    while (1) {
        hw_show();

        usleep(1000);

        // Flicker
        coroutines_goto_next_task (task_table[num_channels], task_table[0]);
        cycle_counter ++;
    }
}

