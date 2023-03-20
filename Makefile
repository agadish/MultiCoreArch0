CC=/usr/bin/g++
CFLAGS=-O3 -Wall -pedantic -fopenmp
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXEC=parsort

.PHONY: clean all

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)
