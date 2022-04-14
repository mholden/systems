CC=gcc
CFLAGS=-g
DEVEL=/home/mholden/devel
DS=$(DEVEL)/data_structs
SY=$(DEVEL)/synch
JS=$(DEVEL)/2022-interview-prep/journaled_ods
INCLUDES=-I$(JS)/include -I$(SY)/include -I$(DS)/include

all: on_disk_system

on_disk_system: test_on_disk_system.c ../on_disk_system.c ../include/on_disk_system.h ../bcache.c ../include/bcache.h \
					../journal.c ../include/journal.h ../transaction.c ../include/transaction.h ../ods_block.c \
					../include/ods_block.h ../block.c ../include/block.h
	$(CC) $(CFLAGS) $(INCLUDES) test_on_disk_system.c ../on_disk_system.c ../bcache.c ../journal.c ../transaction.c \
		../ods_block.c ../block.c $(SY)/synch.c -pthread -o test_on_disk_system

clean:
	rm *o test_on_disk_system

.PHONY: all clean
