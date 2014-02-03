#!/bin/bash

cd examples

# Test reading file data
echo ""
echo "========================================"
echo " Building SD speed program\n\n"
huc -DPRINTFUNCS -s -O2 test-speed.c && pceas -s -l0 test-speed.s
cd -
