CC = gcc
CFLAGS = -Wall -O2 -g

.PHONY: all
all: echoserveri echoserverp echoservert echoservert_pre

echoserveri: echoserveri.c echo.c
	$(CC) $(CFLAGS) -o echoserveri echoserveri.c echo.c -lpthread -lm

echoserverp: echoserverp.c echo.c
	$(CC) $(CFLAGS) -o echoserverp echoserverp.c echo.c -lpthread -lm

echoservert: echoservert.c echo.c
	$(CC) $(CFLAGS) -o echoservert echoservert.c echo.c -lpthread -lm

echoservert_pre: echoservert_pre.c sbuf.c echo.c echo_cnt.c
	$(CC) $(CFLAGS) -o echoservert_pre echoservert_pre.c sbuf.c echo.c echo_cnt.c -lpthread -lm

.PHONY: clean
clean:
	rm -f echoserveri echoserverp echoservert echoservert_pre
