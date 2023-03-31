CC = gcc
CCFLAGS = -g

all: mysh

myshS.c: myshS.c
	$(CC) $(CCFLAGS) -c myshS.c

clean:
	rm -f myshS myshS.c