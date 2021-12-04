// candle for 24 LED ring
// based on 8 pixel version by Tim Bartlett, December 2013
// Modified by Jack Whitham for Adafruit Circuit Playground Express board.
// Modified again to use the rainbow mode only depending on switch setting.
// Modified again for an unbranded 2812B LED ring driven by an Arduino Nano;
// I was not able to use the coroutines this time due to low RAM.
//

// Switch: middle position is maximum brightness, left is night light, right is stars.

#include <stdlib.h>

static char name_text[] = "made by Jack Whitham using lots of borrowed code! this version from 04/12/21";

#include <Adafruit_NeoPixel.h>

#define NUM_LEDS                    24
#define NEOPIXEL_PIN                PD2
#define SWITCH_PIN_COMMON           PD5
#define SWITCH_PIN_LEFT             PD4
#define SWITCH_PIN_RIGHT            PD6
#define COLOUR_ROTATION_TIME        19997 /* milliseconds for a full rotation */
#define STARS_ROTATION_TIME         29989 /* milliseconds for a full rotation */

static Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

static unsigned long tick_time = 0;


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

static bool is_stars_mode(void)
{
    return !digitalRead(SWITCH_PIN_RIGHT);
}

static bool is_night_mode(void)
{
    return !digitalRead(SWITCH_PIN_LEFT);
}

static int ch_constrain(int x, int l, int h)
{
    if (x < l) return l;
    if (x > h) return h;
    return x;
}

static void set_rgb(int channel, int r, int g, int b, int brightness)
{
    brightness = brightness >> 1;
    r = ch_constrain((r * brightness) >> 7, 0, 255);
    g = ch_constrain((g * brightness) >> 7, 0, 255);
    b = ch_constrain((b * brightness) >> 7, 0, 255);
    strip.setPixelColor(channel, r, g, b);
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
static unsigned compute_channel_0_pos()
{
    return (((unsigned long) tick_time << (unsigned long) 16)
            / (unsigned long) COLOUR_ROTATION_TIME);
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

// the light rotates around the ring
static void stars(int brightness)
{
    unsigned long pos = tick_time;

    // period of rotation for the light
    pos %= (unsigned long) STARS_ROTATION_TIME;

    // where is the light (as a fixed point value, which
    // is the LED number 0..23 multiplied by 256)
    pos *= ((unsigned long) 256 * (unsigned long) NUM_LEDS);
    pos /= (unsigned long) STARS_ROTATION_TIME;

    unsigned channel = pos / 256;
    unsigned fraction = pos % 256;

    // Get the colour wheel position now
    unsigned wheel_pos = compute_channel_0_pos();

    // Get RGB colour
    unsigned r, g, b;
    compute_wheel_rgb(wheel_pos >> 8, &r, &g, &b);

    // Brightness within a smaller range
    brightness = ch_constrain(brightness, 64, 127);

    // All unused LEDs are off
    unsigned i;
    for (i = 0; i < NUM_LEDS; i++) {
        set_rgb(i, 0, 0, 0, 0);
    }
    // The lower of the two used LEDs is on
    set_rgb(channel, r, g, b, (brightness * (255 - fraction)) / 128);
    // Upper LED on too
    channel = (channel + 1) % NUM_LEDS;
    set_rgb(channel, r, g, b, (brightness * fraction) / 128);
}

static void wait_for_tick()
{
    unsigned long last_time = tick_time;
    tick_time = millis();
    while ((tick_time - last_time) < 2) {
        tick_time = millis();
    }
}

static void set_light(int brightness, int f_delay)
{
    while(f_delay > 0) {
        f_delay -= 2;
        if (is_stars_mode()) {
            stars(brightness);
        } else if (is_night_mode()) {
            rainbow(brightness / 4);
        } else {
            rainbow(brightness);
        }
        wait_for_tick();
        strip.show();
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

static void fade_in()
{
    for (int green_pixel = 0; green_pixel < green_high; green_pixel ++) {
        set_light(green_pixel, 40);
    }
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
    Serial.begin(57600);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(NEOPIXEL_PIN, OUTPUT);
    pinMode(SWITCH_PIN_COMMON, OUTPUT);
    pinMode(SWITCH_PIN_LEFT, INPUT_PULLUP);
    pinMode(SWITCH_PIN_RIGHT, INPUT_PULLUP);
    digitalWrite(LED_BUILTIN, 0);
    digitalWrite(SWITCH_PIN_COMMON, 0);
    strip.begin();
    strip.show();
    Serial.println("");
    Serial.println(name_text);
    srand(analogRead(A0));
    wait_for_tick();
    fade_in();
}
