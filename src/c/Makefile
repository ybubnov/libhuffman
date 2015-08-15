.PHONY: all, clean

NAME    := hrc

BUILD   := build

INCLUDE := -Iinclude
INCLUDE += -Iinclude/runtime

SOURCE  := src/*.c
SOURCE  += src/huffman/*.c

CFLAGS  := -Wall -Wpedantic -Werror -O2 $(INCLUDE) -std=c99

# Object file names
OBJECTS := $(wildcard $(SOURCE))
OBJECTS := $(subst .c,.o,$(OBJECTS))

CC      := gcc

all: $(BUILD) $(BUILD)/$(NAME)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/$(NAME): main.c $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD)
	rm -rf $(OBJECTS)
