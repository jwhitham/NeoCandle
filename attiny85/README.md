This is a candle flicker animation for ATTiny85 combined with the
[3D RGB Xmas Tree](https://thepihut.com/products/3d-rgb-xmas-tree-for-raspberry-pi])
from The Pi Hut. The LEDs are driven by SPI (they are not WS2812B).

![sample3](/sample3.jpg)

The 3D RGB Xmas Tree header is supposed to plug into a Raspberry Pi,
but can be connected to any microcontroller like this:

- MOSI (Broadcom GPIO 12 on Pi) - Physical pin 32
- CLK (Broadcom GPIO 25 on Pi) - Physical pin 22
- 5V - Physical pin 2
- GND - Physical pin 39

For ATTiny85 the connections are

- MOSI (PB1) - Physical pin 6
- CLK (PB2) - Physical pin 7
- 5V - Physical pin 8
- GND - Physical 4
- Switch left (PB3) - Physical pin 2 - disable candle flicker
- Switch right (PB4) - Physical pin 3 - disable rainbow colours
- LED - Physical pin 5 - flashes at 2Hz when the software is working ok

Pins 2, 3 and 5 don't need to be connected and pin 1 (reset) should
be left unconnected.

This version does not use coroutines due to lack of RAM. The tinySPI library is used
to drive the LEDs, see http://github.com/JChristensen/tinySPI

The candle simulation is based on code by Tim Bartlett, see http://timbartlett.net/ledcandle/

Build this with the Arduino IDE.

