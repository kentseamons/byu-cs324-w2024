CC = gcc
CFLAGS = -Wall -O2 -g

all: proxy
.PHONY: all

proxy: proxy.c
	$(CC) $(CFLAGS) -o proxy proxy.c

.PHONY: clean
clean:
	rm -f proxy
