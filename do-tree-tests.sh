#!/bin/bash
clear && g++-12 -Wall -pedantic -std=c++17 -Iinc unit-tests/sample-tree-test-suite.cpp -o ./unit-tests/.tester && ./unit-tests/.tester && echo "Tester tester successful!"
