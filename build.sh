#!/bin/bash

cd src
huc -DDEBUG -s -O2 test.c && pceas -s -l0 test.s
cd -
