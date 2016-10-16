#!/bin/bash
./main
convert output.ppm output.png > /dev/null
eom output.png > /dev/null
