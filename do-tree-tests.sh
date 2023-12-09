#!/bin/bash
clear && g++-12 -std=c++17 -I../inc unit-tests/sample-tree-test-suite.cpp -o .unit-tests/.tester && ./unit-tests/.tester && echo "Tester tester successful!"
