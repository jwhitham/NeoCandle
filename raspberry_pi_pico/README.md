This candle flicker animation is for RPi Pico or Pico W.
It's intended to be used with the
[3D RGB Xmas Tree](https://thepihut.com/products/3d-rgb-xmas-tree-for-raspberry-pi])
from The Pi Hut. The LEDs are driven by SPI (they are not WS2812B).
The header is supposed to plug into a RPi 2, 3 or 4 but can be
connected to any microcontroller like this:

- MOSI (Broadcom GPIO 12 on Pi) - Physical pin 32
- CLK (Broadcom GPIO 25 on Pi) - Physical pin 22
- 5V - Physical pin 2
- GND - Physical pin 39
