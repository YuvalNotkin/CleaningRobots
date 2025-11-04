CXX = g++
CXXFLAGS = -std=c++17 -Wall -Isrc
SRC = $(shell find src -name '*.cpp')
BIN = build/app

.PHONY: all clean run

all: run

$(BIN): $(SRC)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

clean:
	rm -rf build
run: $(BIN)
	./$(BIN)
