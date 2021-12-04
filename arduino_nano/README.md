This is a candle flicker animation for Arduino Nano combined with
a ring of 24 WS2812B LEDs. These are known as "NeoPixels" by Adafruit,
but are available as generic components too. The combination of a Nano
and a generic WS2812B ring is cheaper than a genuine Adafruit CPE board,
though there are fewer hardware features.

This version does not use coroutines. The Adafruit libraries
are used to drive the LEDs.

The candle simulation is based on code by Tim Bartlett, see http://timbartlett.net/ledcandle/

Build this with the Arduino IDE. The Adafruit Circuit Playground library is
required.

