.PHONY: all, clean, debug, test

CFLAGS = -Wall -O2
SPATH = src/
CC = gcc

all: main

clean:
	rm -rf $(SPATH)huffman.o huffmand.o main maind

test: main test.file
	@MSEC_START=$$(( $$(date +%s%N) / 1000000 )); \
	./main -c test.file test_compressed.file; \
	./main -x test_compressed.file test_uncompressed.file; \
	MSEC_END=$$(( $$(date +%s%N) / 1000000 )); \
	echo "scale=10; ($$MSEC_END - $$MSEC_START) / 1000.0" | bc
	@md5sum test.file
	@md5sum test_uncompressed.file

main: $(SPATH)main.c $(SPATH)huffman.o
	$(CC) $^ -o $@

huffman.o: $(SPATH)huffman.c
	$(CC) -c $< -o $@

huffmand.o: $(SPATH)huffman.c
	$(CC) -c -g $< -o $@

huffman.c: $(SPATH)huffman.h

