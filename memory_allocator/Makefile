CC=gcc
CFLAGS=-g
DEVEL=/home/mholden/devel
MA=$(DEVEL)/2022-interview-prep/memory_allocator
SY=$(DEVEL)/synch
INCLUDES=-I$(MA)/include -I$(SY)/include

all: memory_allocator

memory_allocator: memory_allocator.c include/memory_allocator.h page_allocator.c include/page_allocator.h test_memory_allocator.c
	$(CC) $(CFLAGS) $(INCLUDES) memory_allocator.c page_allocator.c test_memory_allocator.c $(SY)/synch.c -pthread -o test_memory_allocator

clean:
	rm *o test_memory_allocator

.PHONY: all clean

