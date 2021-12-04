This is a candle flicker animation for a Raspberry Pi 1 combined with
a ring of 24 WS2812B LEDs. These are known as "NeoPixels" by Adafruit,
but are available as generic components too.

The WS2812B driver from Jeremy Garff https://github.com/jgarff/rpi\_ws281x
is used to drive the LEDs. The control program runs in place of
/sbin/init. This may only work with a Raspberry Pi 1 (i.e. first
generation, the one with the 26-pin header and the full-size SD card
slot). Newer models require different Linux kernels and might have
a different pinout. However the program is probably compatible and
the WS2812B driver seems to support RPis up to version 4.

The candle simulation is based on code by Tim Bartlett, see http://timbartlett.net/ledcandle/

Use a Linux distribution running on a Raspberry Pi to build this.
Compile the WS2812B driver with "scons" and then run "build.sh"
to build the control program. Then copy the "boot" files to the
root directory of an SD card formatted with FAT16. No
Linux-specific root filesystem is needed on the SD card.
Connect the WS2812B ring as follows:

    5V to RPi pin 2
    GND to RPi pin 6
    DI to RPi pin 12 (aka. "GPIO18", "PCM_CLK")

Strictly speaking a level-shifter is required to convert the 3.3V
output of the RPi to the 5V TTL input required by the WS2812B ring.
I was fortunate and found this was not necessary. YMMV.
