CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
BUILD := build
CORE := src/protocol.cpp src/asset_tracker.cpp

.PHONY: all demo test clean

all: demo

$(BUILD):
	mkdir -p $(BUILD)

demo: $(BUILD)
	$(CXX) $(CXXFLAGS) -Isrc $(CORE) src/lab_demo.cpp -o $(BUILD)/rf-lab
	./$(BUILD)/rf-lab

test: $(BUILD)
	$(CXX) $(CXXFLAGS) -Isrc $(CORE) tests/protocol_test.cpp -o $(BUILD)/protocol-test
	./$(BUILD)/protocol-test

clean:
	rm -rf $(BUILD)
