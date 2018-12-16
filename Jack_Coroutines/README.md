Coroutines library for Cortex-M0 CPU by Jack Whitham

This is based on the code at https://fanf.livejournal.com/105413.html
but because coroutines involve some pretty weird stuff which GCC
will not expect, I felt more comfortable translating the code to Thumb
assembly rather than relying on the Arduino IDE's version of GCC to
do the right thing.


