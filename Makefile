CXX = g++
LD = g++
CXXFLAGS = -std=c++17 -I./inc -g -Wall -pedantic -Werror -fno-pretty-templates -DMACRO_PLATFORM_LINUX
SOURCES  = $(shell find src | grep '[.]cpp$$')
OBJECTS_DIR = .debug-build
OUTPUT = libcommon-d.a
OBJECTS = $(addprefix $(OBJECTS_DIR)/,$(SOURCES:%.cpp=%.o))

$(OBJECTS_DIR)/%.o: | $(OBJECTS_DIR)/src
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $@

default: $(OBJECTS)
	@echo $(SOURCES)
	ar rvs $(OUTPUT) $(OBJECTS)

$(OBJECTS_DIR)/src:
	mkdir -p $(OBJECTS_DIR)/src

clean:
	rm -rf $(OBJECTS_DIR)

libcommon-d.a : default

test: $(OUTPUT)
	$(CXX) $(CXXFLAGS) -L$(shell pwd) unit-tests/TestFixedLengthArray.cpp -lcommon-d -o unit-tests/.utfla
	$(CXX) $(CXXFLAGS) -L$(shell pwd) unit-tests/TestUtil.cpp -lcommon-d -o unit-tests/.tu
	$(CXX) $(CXXFLAGS) -L$(shell pwd) unit-tests/TestMultiType.cpp -lcommon-d -o unit-tests/.tmt
	$(CXX) $(CXXFLAGS) -L$(shell pwd) unit-tests/TestGrid.cpp -lcommon-d -o unit-tests/.tg
	$(CXX) $(CXXFLAGS) -L$(shell pwd) unit-tests/test-parse-options.cpp -lcommon-d -o unit-tests/.tpo
	$(CXX) $(CXXFLAGS) -L$(shell pwd) unit-tests/test-string-utils.cpp -lcommon-d -o unit-tests/.tsu
	./unit-tests/.utfla
	./unit-tests/.tu
	./unit-tests/.tmt
	./unit-tests/.tg
	./unit-tests/.tsu
	./unit-tests/test-po.sh

