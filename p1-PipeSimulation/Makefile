# Specify compiler
CC = g++

# Flags for compiler:
#  -Wall  - turn on compiler warnings
CFLAGS = -Wall -std=c++11

# Flags for program exec.
XFLAGS = -d /Users/jordanball/testZone

OBJECTS = main.o CommandOptions.o

TARGET = main

.PHONY: main
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)
	rm -f $(OBJECTS)
# ./$(TARGET) $(XFLAGS)