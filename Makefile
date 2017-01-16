CC = gcc-6
CXX = g++-6

.PHONY: all, clean

all: wde

wde: wde.c map.c
	$(CC) -std=c11 wde.c map.c -o wde -Wall

clean:
	rm -f wde
	rm -f wdepp
