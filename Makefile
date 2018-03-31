TARGET=Test
LIBS=-lboost_thread -lboost_system -lrt -lpthread
CC=g++
CFLAGS=-std=c++11 -g -Wall
CFLAGS=-std=c++11 -g -Wall -O3

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS=$(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS=$(wildcard *.h) Makefile

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
