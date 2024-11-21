// candle for 25 LED Christmas Tree
// based on 8 pixel version by Tim Bartlett, December 2013
// Modified by Jack Whitham for Adafruit Circuit Playground Express board.
// Modified again to use the rainbow mode only depending on switch setting.
// Modified again for an unbranded 2812B LED ring driven by an Arduino Nano;
// I was not able to use the coroutines this time due to low RAM.
// Modified again for a 3D RGB Xmas Tree driven by ATTiny85 (uses SPI).
//


#include <stdlib.h>

static char name_text[] =
"made by Jack Whitham using lots of borrowed code! this version from 21/11/24\n"
"https://github.com/jwhitham/NeoCandle/\n";

#include <tinySPI.h>

#define NUM_LEDS                    25
#define MOSI_PIN 1      // physical pin 6
#define SCLK_PIN 2      // physical pin 7
#define LED_PIN 0       // physical pin 5
#define SWITCH_PIN_LEFT 3    // physical pin 2
#define SWITCH_PIN_RIGHT 4    // physical pin 3
#define COLOUR_ROTATION_TIME        19997 /* milliseconds for a full rotation */
#define STARS_ROTATION_TIME         29989 /* milliseconds for a full rotation */

static int16_t periodic_tick_time = 0;
static uint32_t millisecond_counter = 0;


static const int green_high = 70;

static const float candleness = 1.0; // amount of flickeriness

// animation time variables, with recommendations
static const int burn_depth = 6 * candleness; //how much green dips below green_high for normal burn - 
static const int flutter_depth = 20 * candleness; //maximum dip for flutter
static const int cycleTime = 120; //duration of one dip in milliseconds

// pay no attention to that man behind the curtain
static const int flicker_depth = (burn_depth + flutter_depth) / 2.4;
static const int burn_low = green_high - burn_depth;
static const int burn_delay = (cycleTime / 2) / burn_depth;
static const int flick_low = green_high - flicker_depth;
static const int flick_delay = (cycleTime / 2) / flicker_depth;
static const int flut_low = green_high - flutter_depth;
static const int flut_delay = ((cycleTime / 2) / flutter_depth);

#define START_OF_FRAME_SIZE 4
#define END_OF_FRAME_SIZE   5
#define SPI_DATA_SIZE         (START_OF_FRAME_SIZE + END_OF_FRAME_SIZE + (4 * NUM_LEDS))
static uint8_t spi_data[SPI_DATA_SIZE];
static uint8_t remap[NUM_LEDS] = {
                    //  Pi Hut 3D Christmas tree (TPH-017)
                    //  LEDs are connected via SPI and numbered from 0
        19, 20, 21, //  1 O'Clock - 19 bottom, 20, 21 top
        7, 8, 9,    //  2 O'Clock - 7 bottom, 8, 9 top
        0, 1, 2,    //  4 O'clock - 0 bottom, 1, 2 top
        16, 17, 18, //  5 O'Clock - 16 bottom, 17, 18 top
        3,          //  Top
        15, 14, 13, //  7 O'Clock - 13 top, 14, 15 bottom
        6, 5, 4,    //  8 O'Clock - 4 top, 5, 6 bottom
        12, 11, 10, //  10 O'Clock - 10 top, 11, 12 bottom
        24, 23, 22, //  11 O'Clock - 22 top, 23, 24 bottom
        };

static bool is_candle_mode(void)
{
    return !digitalRead(SWITCH_PIN_RIGHT);
}

static bool is_rainbow_mode(void)
{
    return !digitalRead(SWITCH_PIN_LEFT);
}

static int ch_constrain(int x, int l, int h)
{
    if (x < l) return l;
    if (x > h) return h;
    return x;
}

static void set_rgb(uint8_t channel, int16_t r, int16_t g, int16_t b, int16_t brightness)
{
    if (channel >= NUM_LEDS) {
        return;
    }
    // in order to ensure that negative numbers are clamped to zero rather than
    // 255, we use signed integers here, but this does mean that the brightness
    // multiplication must be divided by 2 in order to avoid an overflow.
    brightness = brightness >> 1;
    r = ch_constrain((r * brightness) >> 7, 0, 255);
    g = ch_constrain((g * brightness) >> 7, 0, 255);
    b = ch_constrain((b * brightness) >> 7, 0, 255);
    channel = remap[channel];
    spi_data[START_OF_FRAME_SIZE + (4 * channel)] = 0xff; // max brightness set in LED
    spi_data[START_OF_FRAME_SIZE + (4 * channel) + 1] = b;
    spi_data[START_OF_FRAME_SIZE + (4 * channel) + 2] = g;
    spi_data[START_OF_FRAME_SIZE + (4 * channel) + 3] = r;
}

static int approx (int midpoint)
{
    return ch_constrain (midpoint + (rand () % 5) - 2, 1, 20);
}


// Assuming an 0..255 colour wheel position, compute r, g, b channel values
static void compute_wheel_rgb(unsigned wheel_pos,
                              unsigned *r, unsigned *g, unsigned *b)
{
    if(wheel_pos < 85) {
        *r = 255 - wheel_pos * 3;
        *g = 0;
        *b = wheel_pos * 3;
    } else {
        if(wheel_pos < 170) {
            wheel_pos -= 85;
            *r = 0;
            *g = wheel_pos * 3;
            *b = 255 - wheel_pos * 3;
        } else {
            wheel_pos -= 170;
            *r = wheel_pos * 3;
            *g = 255 - wheel_pos * 3;
            *b = 0;
        }
    }
}

// Compute an 0..65535 colour wheel position based on the tick time
static uint16_t compute_channel_0_pos()
{
    uint32_t pos = millisecond_counter;

    // period of rotation for the colour
    pos %= (uint32_t) COLOUR_ROTATION_TIME;

    // fixed point arithmetic to get a value in the 0..65535 range
    pos <<= (uint32_t) 16;
    pos /= (uint32_t) COLOUR_ROTATION_TIME;
    return (uint16_t) pos;
}

// Output: the whole ring shows a rotating rainbow effect at the given brightness
static void rainbow(int brightness)
{
    unsigned char channel;

    // Wheel position for channel 0
    unsigned wheel_pos = compute_channel_0_pos();

    for (channel = 0; channel < NUM_LEDS; channel++) {
        // Input a value 0 to 255 to get a color value.
        // The colours are a transition r - g - b - back to r.
        unsigned r, g, b;
        compute_wheel_rgb(wheel_pos >> 8, &r, &g, &b);
        set_rgb(channel, r, g, b, brightness);
        wheel_pos += ((unsigned long) 1 << (unsigned long) 16) / (unsigned long) NUM_LEDS;
    }
}

// Output: yellow candle effect
static void candle(int brightness)
{
    unsigned char channel;
    static const int redPx = 200;
    static const int grnPx = green_high;
    static const int bluePx = 5;

    for (channel = 0; channel < NUM_LEDS; channel++) {
        set_rgb(channel, redPx, grnPx, bluePx, ch_constrain(brightness + (rand() % 7) - 3, 0, 255));
    }
}


static int wait_for_tick()
{
    // Usually all work can be completed in 3 milliseconds
    // but every few seconds there will be a deadline miss (causing the
    // LED on physical pin 5 to flash).
    // Use 4 milliseconds to avoid this.
    const int ms = 4;
    int16_t current_time;
    periodic_tick_time += ms * 1000;
    millisecond_counter += ms;
    uint8_t deadline_not_met = 2;
    do {
        current_time = (int16_t) micros();
        deadline_not_met = deadline_not_met >> 1;
    } while ((current_time - periodic_tick_time) < 0);
    digitalWrite(LED_PIN, deadline_not_met);
    return ms;
}

static void set_light(int brightness, int f_delay)
{
    while(f_delay > 0) {
        if (is_candle_mode()) {
            candle(brightness);
        } else if (is_rainbow_mode()) {
            rainbow(120);
        } else {
            rainbow(brightness);
        }
        f_delay -= wait_for_tick();
        for (uint8_t i = 0; i < SPI_DATA_SIZE; i++) {
            SPI.transfer(spi_data[i]);
        }
    }
}


static void fire(int green_low, int f_delay)
{
    int green_pixel;
    for (green_pixel = green_high; green_pixel > green_low; green_pixel--) {
        set_light(green_pixel, f_delay);
    }  
    for (green_pixel = green_low; green_pixel < green_high; green_pixel++) {
        set_light(green_pixel, f_delay);
    }
}

static void on(int f)
{
    int green_pixel = green_high - 6;
    set_light(green_pixel, f * 100);
}

static void burn(int f)
{
    const int f_rep = f * 4;
    const int f_delay = burn_delay;
    for (int var = 0; var < f_rep; var++) {
        fire(burn_low, f_delay);
    }  
}

static void flicker(int f)
{
    const int f_rep = f * 4;
    int f_delay = burn_delay;
    fire(burn_low, f_delay);
    f_delay = flick_delay;
    for (int var = 0; var < f_rep; var++) {
        fire(flick_low, f_delay);
    }
    f_delay = burn_delay;
    fire(burn_low, f_delay);
    fire(burn_low, f_delay);
    fire(burn_low, f_delay);
}

static void flutter(int f)
{
    const int f_rep = f * 4;  
    int f_delay = burn_delay;
    fire(burn_low, f_delay);
    f_delay = flick_delay;
    fire(flick_low, f_delay);
    f_delay = flut_delay;
    for (int var = 0; var < f_rep; var++) {
        fire(flut_low, f_delay);
    }
    f_delay = flick_delay;
    fire(flick_low, f_delay);
    fire(flick_low, f_delay);
    f_delay = burn_delay;
    fire(burn_low, f_delay);
    fire(burn_low, f_delay);
}


void loop (void)
{
    int state = rand() % 10;
    switch (state) {
        case 0:
            burn(approx(10));
            break;
        case 1:
            flicker(approx(5));
            break;
        case 2:
            burn(approx(8));
            break;
        case 3:
            flutter(approx(6));
            break;
        case 4:
            burn(approx(3));
            break;
        case 5:
            on(approx(15));
            break;
        case 6:
            burn(approx(10));
            break;
        case 7:
            flicker(approx(10));
            break;
        case 8:
            on(approx(5));
            break;
        default:
            flutter(approx(10));
            break;
    }
}

void setup(void)
{
    SPI.begin();
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, 0);
    pinMode(SWITCH_PIN_LEFT, INPUT_PULLUP);
    pinMode(SWITCH_PIN_RIGHT, INPUT_PULLUP);
    memset(spi_data, 0, SPI_DATA_SIZE);
    srand(1);
    periodic_tick_time = (int16_t) micros();
    millisecond_counter = 0;
}
