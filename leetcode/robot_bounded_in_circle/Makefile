CC=gcc
CFLAGS=-g
INCLUDES=

all: rbic

rbic: test_rbic.c rbic.c
	$(CC) $(CFLAGS) $(INCLUDES) test_rbic.c rbic.c -o test_rbic

clean:
	rm *o test_rbic

.PHONY: all clean
