CXX=/usr/bin/g++
CXXFLAGS_BASIC=-O3 -Wall -pedantic -std=c++17 -Werror
CXXFLAGS_CILK=-fcilkplus
CXXFLAGS=$(CXXFLAGS_BASIC) $(CXXFLAGS_CILK)
LIBRARIES=cilkrts
LDFLAGS=$(addprefix $(LIBRARIES),-l)
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXEC=parsort

.PHONY: clean all

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)
