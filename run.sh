#!/bin/bash
./main
convert output.ppm output.png 2> /dev/null
eom output.png 2> /dev/null
