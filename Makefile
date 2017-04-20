SRC=./src

CC=gcc
CFLAGS=-Wall -O1

.PHONY: projc clean FORCE

FORCE:

projc: FORCE
	$(CC) -o projc $(SRC)/projc.c $(CFLAGS)

clean:
	rm *.obj *.o
