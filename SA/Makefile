CC=g++
LDFLAGS=-std=c++20 -O3 -lm
SOURCES=src/partitioner.cpp src/main.cpp
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=fm
INCLUDES=src/cell.h src/net.h src/partitioner.h
CXX = mpic++

all: $(SOURCES) bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDES}
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o bin/$(EXECUTABLE)
