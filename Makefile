CC = gcc-6
CFLAGS = -Wall -std=c11 -O3

.PHONY: clean check

wde: wde.c map.c
	$(CC) $(CFLAGS) $? -o $@

check: tests.c map.c
	@$(CC) -lcheck $(CFLAGS) -Wno-main $? -o tests
	@./tests

clean:
	rm -f wde
	rm -f tests
