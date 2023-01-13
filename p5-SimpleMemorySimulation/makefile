SYS := $(shell g++ -dumpmachine)
ifneq (, $(findstring apple, $(SYS)))
CFLAGS	= -g -Wall --pedantic -std=c++11 
LFLAGS	= -lpthread
else
CFLAGS	= -g -Wall --pedantic -std=c++11
LFLAGS	= -lpthread
endif

CC		= g++

source_s = main.cpp mmu.cpp tlb.cpp
obj_s  = $(source_s:.cpp=.o)
deps_s = $(source_s:.cpp=.d)

a.out: $(source_s)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) *.o $(deps_s) a.out core


-include $(deps_s)