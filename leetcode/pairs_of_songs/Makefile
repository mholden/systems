CC=gcc
CFLAGS=-g
INCLUDES=

all: pos

pos: test_pos.c pos.c
	$(CC) $(CFLAGS) $(INCLUDES) test_pos.c pos.c -o test_pos

clean:
	rm *o test_pos

.PHONY: all clean
