// candle for Adafruit NeoPixel
// based on 8 pixel version by Tim Bartlett, December 2013
// Modified by Jack Whitham for Adafruit Circuit Playground Express board.
// Compile this within the Arduino IDE.
//
//
// To test this code as a model on Raspberry Pi:
// g++ -o x.exe -x c++ cpelight.ino -x assembler -IJack_Coroutines  Jack_Coroutines/jack_coroutines.S  -Wall -g -DMODEL

#include <stdlib.h>
#include <setjmp.h>
#include <jack_coroutines.h>

static char name_text[] = "made by Jack Whitham using lots of borrowed code! this version from 15/12/18";

#ifdef MODEL
#include <stdio.h>
#else
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_CircuitPlayground.h>
#include <Adafruit_Circuit_Playground.h>
#include <Adafruit_NeoPixel.h>
static const int SPEAKER_SHUTDOWN = 11;
static const int tap_threshold = 120;
static Adafruit_CircuitPlayground acp = Adafruit_CircuitPlayground();
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
static Adafruit_CPlay_NeoPixel & strip = acp.strip;
#endif

typedef enum {
    candle = 0,
    red,
    green,
    blue,
    white,
    rainbow_1,
    rainbow_2,
    num_colour_modes} t_colour_mode;

static t_colour_mode colour_mode = candle;
static unsigned cycle_counter = 0;
static unsigned last_tapped_cycle_count = ~0;
static char tapped_flag = 0;


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

static const int num_channels = 10;
#ifdef MODEL
static const int task_stack_space = 4096;    // stack bytes per task
#else
static const int task_stack_space = 512;    // stack bytes per task
#endif

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

#ifdef MODEL
    printf ("set %d to %3d %3d %3d\n", channel, r, g, b);
#else
    strip.setPixelColor(channel, r, g, b);
#endif
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
        //const int halfGrn = grnHigh - ((grnHigh - grnPx) / 2);
        // int darkGrn = grnPx - 70;
        //darkGrn = ch_constrain(darkGrn, 5, 255);
        //set_rgb(channel, redPx-180, darkGrn, 0); // 0: static
        //strip.setPixelColor(0, redPx-180, darkGrn, 0);
        //strip.setPixelColor(1, redPx-180, darkGrn, 0);
        //strip.setPixelColor(2, redPx-120, grnPx-50, bluePx-5);
        //strip.setPixelColor(3, redPx-60, grnPx-35, bluePx-2);
        set_rgb(channel, redPx, grnPx, bluePx, (grnPx * redPx) / grnHigh); // 4:
        //strip.setPixelColor(4, redPx, grnPx, bluePx);
        //strip.setPixelColor(5, redPx, grnPx, bluePx);
        //strip.setPixelColor(6, redPx, halfGrn, bluePx);
        //strip.setPixelColor(7, redPx, grnHigh, bluePx);
        //set_rgb(channel, redPx-60, grnPx-35, bluePx-2); // 3: very red
        yield(channel, fDelay);
    }  
    for (grnPx = grnLow; grnPx < grnHigh; grnPx++) {
        //const int halfGrn = grnHigh - ((grnHigh - grnPx) / 2);
        //int darkGrn = grnPx-70;
        //darkGrn = ch_constrain(darkGrn, 5, 255);
        //set_rgb(channel, redPx-180, darkGrn, 0); // 0: static
        //strip.setPixelColor(0, redPx-180, darkGrn, 0);
        //strip.setPixelColor(1, redPx-180, darkGrn, 0);
        //strip.setPixelColor(2, redPx-120, grnPx-50, bluePx-5);
        //strip.setPixelColor(3, redPx-60, grnPx-35, bluePx-2);
        set_rgb(channel, redPx, grnPx, bluePx, (grnPx * redPx) / grnHigh); // 4:
        //strip.setPixelColor(4, redPx, grnPx, bluePx);
        //strip.setPixelColor(5, redPx, grnPx, bluePx);
        //strip.setPixelColor(6, redPx, halfGrn, bluePx);
        //strip.setPixelColor(7, redPx, grnHigh, bluePx);
        //set_rgb(channel, redPx-60, grnPx-35, bluePx-2); // 3: very red
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
        //set_rgb(channel, redPx-180, grnPx-70, 0); // 0: static
        //strip.setPixelColor(0, redPx-180, grnPx-70, 0);
        //strip.setPixelColor(1, redPx-180, grnPx-70, 0);
        //strip.setPixelColor(2, redPx-120, grnPx-50, bluePx-5);
        //strip.setPixelColor(3, redPx-60, grnPx-35, bluePx-2);
        set_rgb(channel, redPx, grnPx, bluePx, redPx); // 4: 
        //strip.setPixelColor(4, redPx, grnPx, bluePx);
        //strip.setPixelColor(5, redPx, grnPx, bluePx);
        //strip.setPixelColor(6, redPx, grnPx, bluePx);
        //strip.setPixelColor(7, redPx, grnHigh, bluePx);
        //set_rgb(channel, redPx-60, grnPx-35, bluePx-2); // 3: very red
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

// adafruit effect
void rainbow(unsigned cycle_counter, unsigned scale)
{
    unsigned channel;

    for (channel = 0; channel < num_channels; channel++) {
        // Input a value 0 to 255 to get a color value.
        // The colours are a transition r - g - b - back to r.
        unsigned char WheelPos =
            (((channel * 256) / ((unsigned) num_channels * scale)) +
                cycle_counter) & 255;
        if(WheelPos < 85) {
            set_rgb(channel, 255 - WheelPos * 3, 0, WheelPos * 3, 0);
        } else {
            if(WheelPos < 170) {
                WheelPos -= 85;
                set_rgb(channel, 0, WheelPos * 3, 255 - WheelPos * 3, 0);
            } else {
                WheelPos -= 170;
                set_rgb(channel, WheelPos * 3, 255 - WheelPos * 3, 0, 0);
            }
        }
    }
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

static void tap_interrupt (void)
{
    if ((cycle_counter - last_tapped_cycle_count) > 300) {
        tapped_flag = 1;
        last_tapped_cycle_count = cycle_counter;
    }
}

void loop (void)
{
    int channel, i;
    int button[3];
    button[0] = button[1] = button[2] = 0;

    for (i = 0; name_text[i]; i++) {
        srand (name_text[i]);
    }
    for (channel = 0; channel < num_channels; channel++) {
        coroutines_setup_task
            (task_table[channel + 1],
             task_table[channel],
             &all_stack_space[((channel + 1) * task_stack_space) / sizeof (long)],
             task_0_to_9, (void *) channel);
    }

#ifdef MODEL
    while (1) {
        printf ("--\n");
        coroutines_goto_next_task (task_table[num_channels], task_table[0]);
    }
#else
    while (1) {
        strip.show ();

        button[0] = button[1];
        button[1] = button[2];
        button[2] = 0;
        if (acp.leftButton()) {
            button[2] |= 1;
        }
        if (acp.rightButton()) {
            button[2] |= 2;
        }
        if (tapped_flag) {
            // accelerometer tapped
            colour_mode = t_colour_mode (((int) colour_mode + 1) % (int) num_colour_modes);
            tapped_flag = 0;
        } else if ((button[2] == 0) && (button[0] != 0) && (button[1] == button[0])) {
            // button pressed and stable
            if (button[0] == 1) {
                colour_mode = t_colour_mode (((int) colour_mode + 1) % (int) num_colour_modes);
            } else if (button[0] == 2) {
                colour_mode = t_colour_mode (((int) colour_mode + (int) num_colour_modes - 1) % (int) num_colour_modes);
            }
        }
        delay (1);
        if (acp.slideSwitch()) {
            // Don't flicker
            const int grnPx = grnHigh - 6;
            int i;
            for (i = 0; i < num_channels; i++) {
                set_rgb(i, redPx, grnPx, bluePx, redPx);
            }
        } else {
            // Flicker
            coroutines_goto_next_task (task_table[num_channels], task_table[0]);
        }
        cycle_counter ++;
    }
#endif
}

#ifdef MODEL
int main (void)
{
    loop ();
    return 1;
}
#else
void setup(void)
{    
    acp.begin(255);
    pinMode(SPEAKER_SHUTDOWN, OUTPUT);  // speaker shutdown
    acp.setAccelRange(LIS3DH_RANGE_2_G);
    acp.setAccelTap(1, tap_threshold);
    attachInterrupt(digitalPinToInterrupt(CPLAY_LIS3DH_INTERRUPT), tap_interrupt, FALLING);
    strip.show();
}
#endif

