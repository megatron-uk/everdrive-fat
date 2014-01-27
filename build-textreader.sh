#!/bin/bash

cd examples

# Test reading file data
echo ""
echo "========================================"
echo " Building text file reader program\n\n"
huc -s -O2 test-textreader.c && pceas -s -l0 test-textreader.s
cd -
