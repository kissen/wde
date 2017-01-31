CC = gcc-4.9 -fdiagnostics-color=auto
CFLAGS = -Wall -std=c11 -O3 -g

.PHONY: check clean

wde: wde.c map.c
	$(CC) $(CFLAGS) $? -o $@

check: tests.c map.c wde.c
	@$(CC) $(CFLAGS) -Wno-main tests.c map.c -o tests -lcheck -lm -lrt -pthread
	@./tests
	@rm -f tests

clean:
	rm -f wde
	rm -f tests
