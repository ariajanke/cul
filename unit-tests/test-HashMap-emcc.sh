#!/bin/bash
emcc -std=c++17 -sNO_DISABLE_EXCEPTION_CATCHING -I../inc test-HashMap.cpp -o .thm
node ./.thm

