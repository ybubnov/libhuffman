.PHONY: all, clean, debug

CFLAGS = -Wall -Werror -g
SPATH = src/
CC = gcc

all: main

debug: maind

clean:
	rm -rf $(SPATH)huffman.o huffmand.o main maind

maind: $(SPATH)main.c huffmand.o
	$(CC) -g $(CFLAGS) $^ -o $@

main: $(SPATH)main.c $(SPATH)huffman.o
	$(CC) $(CFLAGS) $^ -o $@

huffman.o: $(SPATH)huffman.c
	$(CC) $(CFLAGS) -c $< -o $@

huffmand.o: $(SPATH)huffman.c
	$(CC) $(CFLAGS) -c -g $< -o $@

huffman.c: $(SPATH)huffman.h

