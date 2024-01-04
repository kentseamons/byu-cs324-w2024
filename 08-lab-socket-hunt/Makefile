CC = gcc
CFLAGS = -Wall -O2 -g

all: treasure_hunter
.PHONY: all

treasure_hunter: treasure_hunter.c
	$(CC) $(CFLAGS) -o treasure_hunter treasure_hunter.c

.PHONY: clean
clean:
	rm -f treasure_hunter
