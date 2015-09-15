#!/bin/bash

source ../settings.ini

# Test file / directory access
echo ""
echo "========================================"
echo " Building file/dir access test program\n\n"

# Build with debugging enabled
$CC -t -DPRINTFUNCS -DFILEDEBUG -s fileopen.c && $AS -s -l0 fileopen.s

# Build without debugging
#huc -s -O2 test-files.c && pceas -s -l0 test-files.s
