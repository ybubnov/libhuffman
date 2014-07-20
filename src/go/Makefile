.PHONY: all, clean, build_path

NAME = hrc
SRCPATH = ./src
BUILDPATH = ./build
CC = go build

all: build_path $(BUILDPATH)/$(NAME)

clean:
	rm -rf $(BUILDPATH)

build_path:
	@mkdir -p $(BUILDPATH)

$(BUILDPATH)/$(NAME):
	cd $(SRCPATH); \
	export GOPATH=$$(dirname $$(pwd)); \
	$(CC)
	echo $$(pwd)
	mv $(SRCPATH)/$(SRCPATH) $@

