This is a candle flicker animation for various small boards,
always involving the colourful WS2812B LEDs or Adafruit "NeoPixels".
I used this to make various Christmas gifts. The first generation
of these devices always used Adafruit "Circuit Playground Express"
hardware, which is very nice but rather expensive. Later generations
used Arduino Nano boards with generic WS2812B LEDs, and in one
case I reused an old Raspberry Pi. The smallest microcontroller
used in these projects is an ATTiny85, which required no external
components aside from the LEDs themselves.

# Examples

![sample2](/sample2.jpg)

In this case the ring of LEDs is covered by a china tealight holder,
creating an aurora borealis effect as the LEDs cycle through colours.
This used an Arduino Nano.

![sample1](/sample1.jpg)

Here, the ring can be covered by an acrylic dome covered with stars.
The program has two modes: in one, all LEDs are lit and cycle through
colours, while in another, a single LED lights at a time, so the device
acts as a star projector (with no moving parts).

![sample3](/sample3.jpg)

The LEDs are on
[a Christmas tree](https://thepihut.com/products/3d-rgb-xmas-tree-for-raspberry-pi)
and [the program runs on an ATTiny85](/attiny85), programmed using Arduino tools.

