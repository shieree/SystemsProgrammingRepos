CC = gcc
CCFLAGS = -g

all: mysh

mysh.c: mysh.c
	$(CC) $(CCFLAGS) -c mysh.c

clean:
	rm -f mysh mysh.c
