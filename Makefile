CXX = g++
LD = g++
CXXFLAGS = -std=c++14 -I./inc -Wall -pedantic -Werror -DMACRO_PLATFORM_LINUX
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

test: $(OUTPUT)
	$(CXX) $(CXXFLAGS) unit-tests/TestFixedLengthArray.cpp -o unit-tests/.utfla
	$(CXX) $(CXXFLAGS) unit-tests/TestUtil.cpp -o unit-tests/.tu
	./unit-tests/.utfla
	./unit-tests/.tu

