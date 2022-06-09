CC=gcc
CFLAGS=-g
INCLUDES=

all: muot

muot: test_muot.c muot.c
	$(CC) $(CFLAGS) $(INCLUDES) test_muot.c muot.c -o test_muot

clean:
	rm *o test_muot

.PHONY: all clean
