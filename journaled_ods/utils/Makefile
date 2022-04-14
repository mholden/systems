CC=gcc
CFLAGS=-g
DEVEL=/home/mholden/devel
DS=$(DEVEL)/data_structs
SY=$(DEVEL)/synch
JS=$(DEVEL)/2022-interview-prep/journaled_ods
INCLUDES=-I$(JS)/include -I$(SY)/include -I$(DS)/include

all: ods_dump ods_check ods_exercise

ods_dump: ods_dump.c ../on_disk_system.c ../include/on_disk_system.h ../bcache.c ../include/bcache.h \
			../journal.c ../include/journal.h ../transaction.c ../include/transaction.h ../ods_block.c \
			../include/ods_block.h ../block.c ../include/block.h
	$(CC) $(CFLAGS) $(INCLUDES) ods_dump.c ../on_disk_system.c ../bcache.c ../journal.c ../transaction.c \
		../ods_block.c ../block.c $(SY)/synch.c -pthread -o ods_dump
		
ods_check: ods_check.c ../on_disk_system.c ../include/on_disk_system.h ../bcache.c ../include/bcache.h \
			../journal.c ../include/journal.h ../transaction.c ../include/transaction.h ../ods_block.c \
			../include/ods_block.h ../block.c ../include/block.h
	$(CC) $(CFLAGS) $(INCLUDES) ods_check.c ../on_disk_system.c ../bcache.c ../journal.c ../transaction.c \
		../ods_block.c ../block.c $(SY)/synch.c -pthread -o ods_check
		
ods_exercise: ods_exercise.c ../on_disk_system.c ../include/on_disk_system.h ../bcache.c ../include/bcache.h \
			../journal.c ../include/journal.h ../transaction.c ../include/transaction.h ../ods_block.c \
			../include/ods_block.h ../block.c ../include/block.h
	$(CC) $(CFLAGS) $(INCLUDES) ods_exercise.c ../on_disk_system.c ../bcache.c ../journal.c ../transaction.c \
		../ods_block.c ../block.c $(SY)/synch.c -pthread -o ods_exercise

clean:
	rm *o ods_dump ods_check ods_exercise

.PHONY: all clean
