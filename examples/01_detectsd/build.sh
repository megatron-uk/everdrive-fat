#!/bin/bash

source ../settings.ini

# detaile device and filesystem info by enabling debug output on fat libraries
echo ""
echo "========================================"
echo " Building SD test and detection program\n\n"

# BUild with debugging enabled - reccomended for this example
$CC -v -DPRINTFUNCS -DFATDEBUG -s detectsd.c && $AS -s -l0 detectsd.s
