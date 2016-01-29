.PHONY: all, clean

NAME    := hrc
BUILD   := build
INCLUDE := -Iinclude

SOURCE  := src/*.c
SOURCE  += src/huffman/*.c

WARNING := -Wall
WARNING += -Wpedantic
WARNING += -Werror
WARNING += -Wunused

CFLAGS  := $(WARNING) $(INCLUDE) -std=c99 -g

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
