CC = gcc
CCFLAGS = -g

all: mysh

myshS.c: mysh.c
	$(CC) $(CCFLAGS) -c mysh.c

clean:
	rm -f mysh mysh.c
