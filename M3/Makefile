CC = gcc
CFLAGS = -Wall -Wextra

comaper.o: compare.c
	$(CC) $(CFLAGS) -c compare.c -o compare.o

compare_static: compare.o
	$(CC) $(CFLAGS) -static compare.o -o compare_static

compare_dynamic: compare.o
	$(CC) $(CFLAGS) compare.o -o compare_dynamic

all: compare_static compare_dynamic

.Phony: clean compare run test

clean:
	rm -f compare_static compare_dynamic compare.o

run: all
	$(CURDIR)/compare_static
	$(CURDIR)/compare_dynamic

test: compare run 