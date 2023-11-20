#!/usr/bin/env bash

rm -rf obj-intel64
mkdir obj-intel64
make PIN_ROOT=$HOME/src/pin/pin-3.28-98749-g6643ecee5-gcc-linux obj-intel64/pin_driver.so