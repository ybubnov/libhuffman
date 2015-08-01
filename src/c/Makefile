.PHONY: all, clean, test

NAME    := hrc

SOURCE  := src
INCLUDE := include
BUILD   := build

CFLAGS  := -Wall -Werror -O2 -I$(INCLUDE)

# Object file names
OBJECTS := $(wildcard $(SOURCE)/*.c)
OBJECTS := $(subst .c,.o,$(OBJECTS))

CC      := gcc

all: $(BUILD) $(BUILD)/$(NAME)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SOURCE)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD)/$(NAME): main.c $(OBJECTS)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -rf $(BUILD)
	rm -rf $(OBJECTS)

