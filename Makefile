CFLAGS = -Wall -std=c11 -O3
CPPFLAGS = -D_XOPEN_SOURCE=700
LDFLAGS = -pthread

.PHONY: check clean

wde: map.o wde.o
	@echo "LD $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

check: map.o tests.c wde.c
	@$(CC) $(CFLAGS) -Wno-main map.o tests.c -o tests -lcheck -lm -lrt -pthread
	@./tests
	@rm -f tests

%.o: %.c
	@echo "CC $@"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f map.o tests.o wde.o
	rm -f wde
	rm -f tests
