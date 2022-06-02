CC=g++
CFLAGS=-g
INCLUDES=

all: prog1

prog1: test_prog1.cpp prog1.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -std=c++20 test_prog1.cpp prog1.cpp -o test_prog1

clean:
	rm *o test_prog1

.PHONY: all clean
