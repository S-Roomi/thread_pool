CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude -pthread

all: build/test_pool build/benchmark

build:
	mkdir -p build

build/test_pool: src/test_pool.cpp include/thread_pool.hpp | build
	$(CXX) $(CXXFLAGS) src/test_pool.cpp -o build/test_pool

build/benchmark: src/benchmark.cpp include/thread_pool.hpp | build
	$(CXX) $(CXXFLAGS) src/benchmark.cpp -o build/benchmark

test: build/test_pool
	./build/test_pool

bench: build/benchmark
	./build/benchmark 2000000 8

clean:
	rm -rf build

.PHONY: all test bench clean
