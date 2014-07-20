.PHONY: all, clean, test, build_path

NAME = hrc
CFLAGS = -Wall -Werror -O2
SRCPATH = ./src
BUILDPATH = ./build
CC = gcc

all: build_path $(BUILDPATH)/$(NAME)

clean:
	rm -rf $(BUILDPATH)

test: $(BUILDPATH)/$(NAME) test.file
	@MSEC_START=$$(( $$(date +%s%N) / 1000000 )); \
	$(BUILDPATH)/$(NAME) -c test.file test_compressed.file; \
	$(BUILDPATH)/$(NAME) -x test_compressed.file test_uncompressed.file; \
	MSEC_END=$$(( $$(date +%s%N) / 1000000 )); \
	echo "scale=10; ($$MSEC_END - $$MSEC_START) / 1000.0" | bc
	@md5sum test.file
	@md5sum test_uncompressed.file

build_path:
	@mkdir -p $(BUILDPATH)

$(BUILDPATH)/$(NAME): $(SRCPATH)/main.c $(BUILDPATH)/huffman.o
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDPATH)/huffman.o: $(SRCPATH)/huffman.c
	$(CC) -c $< -o $@

$(BUILDPATH)/huffmand.o: $(SRCPATH)/huffman.c
	$(CC) -c -g $< -o $@

huffman.c: $(SRCPATH)/huffman.h

