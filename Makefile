CC = gcc-6

.PHONY: all, clean

all: wde

wde: wde.c map.c
	$(CC) -Wall -std=c11 -O3 wde.c map.c -o wde

clean:
	rm -f wde
