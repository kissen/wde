.PHONY: all, clean

all: wde

wde: wde.cc
	$(CXX) wde.cc -o wde -Wall -fno-exceptions -fdiagnostics-color=auto

clean:
	rm -f wde
