CC=g++
CFLAGS=-g -std=c++20
#INCLUDES=-I./innodb

all: cpp-stl

cpp-stl: cpp-stl.cpp
	$(CC) $(CFLAGS) cpp-stl.cpp -o cpp-stl

clean:
	rm *o cpp-stl

install:
	cp cpp-stl $(HOME)/devel/install/

.PHONY: all clean install
