.PHONY: all, clean

NAME    := hrc

GOPATH  := $(shell pwd)
BUILD   := build

SOURCE  := $(wildcard $(SOURCE)/*.go)
SOURCE  += $(wildcard $(SOURCE)/huffman/*.go)

CC      := go build

all: $(BUILD) $(BUILD)/$(NAME)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/$(NAME): $(SOURCE)
	$(CC) -o $@

clean:
	rm -rf $(BUILD)
