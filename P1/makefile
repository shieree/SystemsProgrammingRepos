CC = gcc
CCFLAGS = -g

all: memgrind

memgrind: mymalloc.o memgrind.o
	$(CC) $(CCFLAGS) -o memgrind mymalloc.o memgrind.o

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) $(CCFLAGS) -c mymalloc.c

memgrind.o: memgrind.c
	$(CC) $(CCFLAGS) -c memgrind.c

clean:
	rm -f memgrind mymalloc.o memgrind.o
