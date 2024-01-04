CC = gcc
CFLAGS = -Wall -O2 -g

.PHONY: all
all: echoservere

echoserveri: echoservere.c
	$(CC) $(CFLAGS) -o echoservere echoservere.c

.PHONY: clean
clean:
	rm -f echoservere
