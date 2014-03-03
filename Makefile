.PHONY: all, clean

CFLAGS = -Wall -Werror
SPATH = src/
CC = gcc

all: main

debug: maind

clean:
	rm -rf huffman.o main maind

maind: main.c huffman.o
	$(CC) -g $(CFLAGS) $^ -o $@

main: $(SPATH)main.c $(SPATH)huffman.o
	$(CC) $(CFLAGS) $^ -o $@

huffman.o: $(SPATH)huffman.c
	$(CC) $(CFLAGS) -c $< -o $@

huffman.c: $(SPATH)huffman.h

