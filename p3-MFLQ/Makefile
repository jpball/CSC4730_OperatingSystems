SYS := $(shell g++ -dumpmachine)
ifneq (, $(findstring apple, $(SYS)))
CFLAGS	= -g --pedantic -Wall -std=c++11 
LFLAGS	= -lpthread
else
CFLAGS	= -g --pedantic -Wall -std=c++11
LFLAGS	= -lpthread
endif

CC		= g++

main: main.o MLFQSch.o
	$(CC) $^ $(LFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) *.o a.out