CXX = g++
LD = g++
CXXFLAGS = -std=c++14 -I./inc

test:
	g++ unit-tests/TestFixedLengthArray.cpp -o fla-test $(CXXFLAGS)
	g++ unit-tests/TestUtil.cpp -o util-test $(CXXFLAGS)
	./fla-test
	./util-test
	rm ./fla-test ./util-test
