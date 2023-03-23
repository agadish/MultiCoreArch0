CXX=/usr/bin/g++
CXXFLAGS=-O3 -Wall -pedantic -std=c++11 -Werror -fopenmp
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXEC=parsort

.PHONY: clean all

all: $(EXEC)

$(EXEC): $(SOURCES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)
