CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./include
SRC = $(wildcard src/*.cpp)
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
