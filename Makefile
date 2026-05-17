CXX ?= g++
CXXFLAGS ?= -std=c++17 -O3 -Wall -Wextra -pedantic
CPPFLAGS ?= -Iinclude
LDFLAGS ?=

TARGET := ms_cflp_ci
SOURCES := src/main.cpp src/Instance.cpp src/Solution.cpp src/HeuristicSolver.cpp src/LocalSearch.cpp src/GreedySolver.cpp src/GraspConstructiveSolver.cpp src/GraspSolver.cpp src/GvnsSolver.cpp src/Utils.cpp
OBJECTS := $(SOURCES:.cpp=.o)

.PHONY: all clean run-greedy run-all

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

src/%.o: src/%.cpp include/*.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run-greedy: $(TARGET)
	./$(TARGET) --instance Instaces_MS-CFLP-CI/Public/wlp01.dzn --algorithm greedy

run-all: $(TARGET)
	./$(TARGET) --instances-dir Instaces_MS-CFLP-CI/Public --algorithm all --grasp-iters 15 --gvns-iters 30 --ls-passes 100 --time-limit 10 --output results/results.csv

clean:
	rm -f $(TARGET) src/*.o
