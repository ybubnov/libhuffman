package main

import (
    "io"
    "os"
    "log"
    "huffman"
)

const (
    HelpString = "Usage: [-c] [-x] ifilename ofilename"
)

func main() {
    var process func(*huffman.Huffman, interface{}, interface{}) error
    var err error

    if len(os.Args) < 4 {
        log.Fatal(HelpString)
    }

    switch os.Args[1] {
    case "-c":
        process = func(h *huffman.Huffman, reader, writer interface{}) error {
            return h.Encode(reader.(io.ReadSeeker), writer.(io.Writer))
        }
    case "-x":
        process = func(h *huffman.Huffman, reader, writer interface{}) error {
            return h.Decode(reader.(io.Reader), writer.(io.Writer))
        }
    default:
        log.Fatal(HelpString)
    }


    var iFile, oFile *os.File
    if iFile, err = os.OpenFile(os.Args[2], os.O_RDONLY, 0666); err != nil {
        log.Fatal(err)
    }

    defer iFile.Close()

    if oFile, err = os.Create(os.Args[3]); err != nil {
        log.Fatal(err)
    }

    defer oFile.Close()

    fileInfo, err := iFile.Stat()
    if err != nil {
        log.Fatal(err)
    }

    if err := process(huffman.New(fileInfo.Size()), iFile, oFile); err != nil {
        log.Fatal(err)
    }
}

