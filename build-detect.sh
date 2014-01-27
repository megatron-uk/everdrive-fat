#!/bin/bash

cd examples

# detaile device and filesystem info by enabling debug output on fat libraries
echo ""
echo "========================================"
echo " Building SD test and detection program\n\n"

# BUild with debugging enabled - reccomended for this example
huc -DPRINTFUNCS -DFATDEBUG -s test-detect.c && pceas -s -l0 test-detect.s

# Without debugging symbols - not especially useful for this example program
#huc -s -O2 test-detect.c && pceas -s -l0 test-detect.s


cd -
