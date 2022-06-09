CC=gcc
CFLAGS=-g
INCLUDES=

all: so

so: test_so.c so.c
	$(CC) $(CFLAGS) $(INCLUDES) test_so.c so.c -o test_so
	
solution: solution.cpp
	g++ solution.cpp -o solution
	
attempt2: test_attempt2.cpp attempt2.cpp
	g++ -g test_attempt2.cpp attempt2.cpp -o attempt2

clean:
	rm *o test_so

.PHONY: all clean
