CC=gcc
CFLAGS=-g
INCLUDES=

all: prog1

prog1: test_prog1.c prog1.c
	$(CC) $(CFLAGS) $(INCLUDES) test_prog1.c prog1.c -o test_prog1

clean:
	rm *o test_prog1

.PHONY: all clean
