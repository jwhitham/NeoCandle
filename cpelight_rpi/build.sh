#!/bin/bash -xe

R=rpi_ws281x
gcc -o boot/program.elf program.c -lws2811 -L$R -I$R -lm -static 

