#!/bin/bash
gpioset -t 0 -d open-drain GPIO7=0 GPIO12=0
gpioset -t 0 -d push-pull GPIO12=1
gpioset -t 0 -d open-drain GPIO7=1
