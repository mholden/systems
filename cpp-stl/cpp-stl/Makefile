CC=g++
CFLAGS=-g -std=c++20
#INCLUDES=-I./innodb

all: cpp-stl-int cpp-stl-class

cpp-stl-int: cpp-stl-int.cpp
	$(CC) $(CFLAGS) cpp-stl-int.cpp -o cpp-stl-int

cpp-stl-class: cpp-stl-class.cpp
	$(CC) $(CFLAGS) cpp-stl-class.cpp -o cpp-stl-class

clean:
	rm *o cpp-stl-int cpp-stl-class

.PHONY: all clean
