OBJS	= main.o ext2fs_print.o
SOURCE	= main.cpp ext2fs_print.c
HEADER	= ext2fs_print.h ext2fs.h
OUT	= je2fs
CC	 = g++
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.cpp
	$(CC) $(FLAGS) main.cpp -std=c++17

ext2fs_print.o: ext2fs_print.c
	$(CC) $(FLAGS) ext2fs_print.c -std=c++17

