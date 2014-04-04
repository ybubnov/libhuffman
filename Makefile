.PHONY: all, clean, debug

CFLAGS = -Wall -O2
SPATH = src/
CC = gcc

all: main

debug: maind

clean:
	rm -rf $(SPATH)huffman.o huffmand.o main maind

maind: $(SPATH)main.c huffmand.o
	$(CC) $^ -o $@

main: $(SPATH)main.c $(SPATH)huffman.o
	$(CC) $^ -o $@

huffman.o: $(SPATH)huffman.c
	$(CC) -c $< -o $@

huffmand.o: $(SPATH)huffman.c
	$(CC) -c -g $< -o $@

huffman.c: $(SPATH)huffman.h

