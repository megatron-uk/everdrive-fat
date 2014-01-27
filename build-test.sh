#!/bin/bash

cd examples

# Test file / directory access
echo ""
echo "========================================"
echo " Building file/dir access test program\n\n"

# Build with debugging enabled
#huc -DPRINTFUNCS -DFILEDEBUG -s test-files.c && pceas -s -l0 test-files.s

# Build without debugging
huc -s -O2 test-files.c && pceas -s -l0 test-files.s

cd -
