CC=g++
CFLAGS=-g
INCLUDES=

all: prog1

prog1: test_prog1.cpp
	$(CC) $(CFLAGS) $(INCLUDES) test_prog1.cpp -o test_prog1
	
alt: alt_solution.cpp
	$(CC) $(CFLAGS) $(INCLUDES) alt_solution.cpp -o alt

clean:
	rm *o test_prog1

.PHONY: all clean
