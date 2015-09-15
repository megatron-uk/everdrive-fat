#!/bin/bash

source ../settings.ini

# Test reading file data
echo ""
echo "========================================"
echo " Building SD speed program\n\n"

$CC -DPRINTFUNCS -s -O2 benchmark.c && $AS -s -l0 benchmark.s
