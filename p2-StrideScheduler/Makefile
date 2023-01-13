# Specify compiler
CC = g++

# Flags for compiler:
#  -Wall  - turn on compiler warnings
CFLAGS = -Wall -std=c++11

# Flags for program exec.
XFLAGS =

OBJECTS = main.o Scheduler.o

TARGET = main

.PHONY: main
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS)
	rm -f $(OBJECTS)
# ./$(TARGET) $(XFLAGS)