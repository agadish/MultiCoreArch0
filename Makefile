CC=/usr/bin/g++
CPPFLAGS=-O3 -Wall -pedantic -fopenmp -std=c++17
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXEC=parsort

.PHONY: clean all

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CPPFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) -c $(CPPFLAGS) $^ -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)
