#!/bin/bash -xe

R=rpi_ws281x
C=../Jack_Coroutines
gcc -o boot/program.elf \
    program.c cpelight.c $C/jack_coroutines.S \
    -lws2811 -L$R -I$R -I$C -lm -static -Wall -Werror -O2

