CC=gcc
CFLAGS=-g
INCLUDES=

all: msan

msan: test_msan.c msan.c
	$(CC) $(CFLAGS) $(INCLUDES) test_msan.c msan.c -o test_msan

attempt2: attempt2.cpp
	g++ -std=c++17 $(CFLAGS) attempt2.cpp -o attempt2

clean:
	rm *o test_msan

.PHONY: all clean
