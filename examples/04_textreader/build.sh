#!/bin/bash

source ../settings.ini

# Test reading file data
echo ""
echo "========================================"
echo " Building text file reader program\n\n"

$CC -s -DPRINTFUNCS -DDEBUG -O2 textreader.c && $AS -s -l0 textreader.s
