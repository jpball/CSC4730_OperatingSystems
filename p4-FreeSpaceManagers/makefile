SYS := $(shell g++ -dumpmachine)
ifneq (, $(findstring apple, $(SYS)))
CFLAGS	= -g --pedantic -Wall -std=c++11 
LFLAGS	= -lpthread
else
CFLAGS	= -g --pedantic -Wall -std=c++11
LFLAGS	= -lpthread
endif

CC		= g++

slab_s = main_s.cpp slab.cpp fsm.cpp
obj_s  = $(slab_s:.cpp=.o)
deps_s = $(slab_s:.cpp=.d)

next_n = main_n.cpp next.cpp fsm.cpp
obj_n  = $(next_n:.cpp=.o)
deps_n = $(next_n:.cpp=.d)

best_b = main_b.cpp best.cpp fsm.cpp
obj_b  = $(best_b:.cpp=.o)
deps_b = $(best_b:.cpp=.d)

all:
	make slab
	make next
	make best
	make clean

slab: main_s.o slab.o fsm.o
	$(CC) $^ -o $@ $(LFLAGS)

next: main_n.o next.o fsm.o
	$(CC) $^ -o $@ $(LFLAGS)

best: main_b.o best.o fsm.o
	$(CC) $^ -o $@ $(LFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

# $(RM) is rm -f by default
clean:
	$(RM) *.o $(deps_s) a.out core
	$(RM) $(deps_n)
	$(RM) $(deps_b)


-include $(deps_s)
-include $(deps_n)
-include $(deps_b)