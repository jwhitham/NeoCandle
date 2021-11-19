// candle for 24 LED ring
// based on 8 pixel version by Tim Bartlett, December 2013
// Modified by Jack Whitham for Adafruit Circuit Playground Express board.
// Modified again to use the rainbow mode only depending on switch setting.
// Modified again for an unbranded 2812B LED ring driven by an Arduino Nano;
// I was not able to use the coroutines this time due to low RAM.
//

#include <stdlib.h>

static char name_text[] = "made by Jack Whitham using lots of borrowed code! this version from 19/11/21";

#include <Adafruit_NeoPixel.h>

#define NUM_LEDS                    24
#define NEOPIXEL_PIN                PD3
#define COLOUR_ROTATION_TIME        20000 /* milliseconds for a full rotation */


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



static int ch_constrain(int x, int l, int h)
{
    if (x < l) return l;
    if (x > h) return h;
    return x;
}

static void set_rgb(int channel, int r, int g, int b, int brightness)
{
    r = ch_constrain((r * brightness) >> 8, 0, 255);
    g = ch_constrain((g * brightness) >> 8, 0, 255);
    b = ch_constrain((b * brightness) >> 8, 0, 255);
    strip.setPixelColor(channel, r, g, b);
}

static int approx (int midpoint)
{
    return ch_constrain (midpoint + (rand () % 5) - 2, 1, 20);
}


// adafruit effect
void rainbow(int brightness)
{
    unsigned char channel;

    // Wheel position for channel 0
    unsigned wheel_pos =
        (((unsigned long) tick_time << (unsigned long) 16) / (unsigned long) COLOUR_ROTATION_TIME);

    for (channel = 0; channel < NUM_LEDS; channel++) {
        // Input a value 0 to 255 to get a color value.
        // The colours are a transition r - g - b - back to r.
        unsigned char copy_wheel_pos = (wheel_pos >> (unsigned) 8);
        if(copy_wheel_pos < 85) {
            set_rgb(channel, 255 - copy_wheel_pos * 3, 0, copy_wheel_pos * 3, brightness);
        } else {
            if(copy_wheel_pos < 170) {
                copy_wheel_pos -= 85;
                set_rgb(channel, 0, copy_wheel_pos * 3, 255 - copy_wheel_pos * 3, brightness);
            } else {
                copy_wheel_pos -= 170;
                set_rgb(channel, copy_wheel_pos * 3, 255 - copy_wheel_pos * 3, 0, brightness);
            }
        }
        wheel_pos += ((unsigned long) 1 << (unsigned long) 16) / (unsigned long) NUM_LEDS;
    }
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
        rainbow(brightness);
        wait_for_tick();
        strip.show();
        digitalWrite(LED_BUILTIN, !(tick_time & 1024));
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
    digitalWrite(LED_BUILTIN, 1);
    strip.begin();
    strip.show();
    Serial.println("");
    Serial.println(name_text);
    srand(analogRead(A0));
    wait_for_tick();
    fade_in();
}
