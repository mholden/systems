CC=g++
CFLAGS=-g
INCLUDES=

all: prog1

prog1: test_prog1.cpp
	$(CC) $(CFLAGS) $(INCLUDES) test_prog1.cpp -o test_prog1
	
alt: alt_solution.cpp
	$(CC) $(CFLAGS) $(INCLUDES) alt_solution.cpp -o alt
	
alt2: alt_solution2.cpp
	$(CC) $(CFLAGS) $(INCLUDES) alt_solution2.cpp -o alt2
	
alt3: alt_solution3.cpp
	$(CC) $(CFLAGS) $(INCLUDES) alt_solution3.cpp -o alt3
	
dp: dp_solution.cpp
	$(CC) $(CFLAGS) $(INCLUDES) dp_solution.cpp -o dp
		
clean:
	rm *o test_prog1 alt alt2 alt3 dp

.PHONY: all clean
