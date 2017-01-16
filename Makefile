CC = gcc-6

.PHONY: all, clean

all: wde wdepp

wde: wde.c map.c
	$(CC) wde.c map.c -o wde -Wall

wdepp: wde.cc
	$(CXX) wde.cc -o wdepp -Wall -fno-exceptions

clean:
	rm -f wde
	rm -f wdepp
