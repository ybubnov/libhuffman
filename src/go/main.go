package main

import (
	"flag"
	"huffman"
	"io"
	"log"
	"os"
)

func main() {
	var process func(*huffman.Huffman, io.ReadSeeker, io.Writer) error

	createFlag := flag.Bool("create", false, "create a new archive")
	extractFlag := flag.Bool("extract", false, "extract files from an archive")
	inputFlag := flag.String("input", "", "input file name")
	outputFlag := flag.String("output", "", "output file name")

	flag.Parse()

	switch {
	case *createFlag:
		process = func(h *huffman.Huffman, r io.ReadSeeker, w io.Writer) error {
			return h.Encode(r, w)
		}
	case *extractFlag:
		process = func(h *huffman.Huffman, r io.ReadSeeker, w io.Writer) error {
			return h.Decode(r, w)
		}
	default:
		flag.Usage()
		os.Exit(1)
	}

	if *inputFlag == "" {
		flag.Usage()
		os.Exit(1)
	}

	if *outputFlag == "" {
		flag.Usage()
		os.Exit(1)
	}

	iFile, err := os.OpenFile(*inputFlag, os.O_RDONLY, 0666)
	if err != nil {
		log.Fatal(err)
	}

	defer iFile.Close()

	oFile, err := os.Create(*outputFlag)
	if err != nil {
		log.Fatal(err)
	}

	defer oFile.Close()

	fileInfo, err := iFile.Stat()
	if err != nil {
		log.Fatal(err)
	}

	err = process(huffman.New(fileInfo.Size()), iFile, oFile)
	if err != nil {
		log.Fatal(err)
	}
}
