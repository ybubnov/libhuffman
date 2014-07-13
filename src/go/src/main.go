package main

import (
    "os"
    "log"
    "huffman"
)

func main() {
    var iFile, oFile *os.File
    var err error

    if iFile, err = os.OpenFile(
                        "/home/kubrick/test/file.txt",
                        os.O_RDONLY, 0666); err != nil {
        log.Fatal(err)
    }

    defer iFile.Close()

    if oFile, err = os.Create(
                        "/home/kubrick/test/out.txt"); err != nil {
        log.Fatal(err)
    }

    defer oFile.Close()

    huffman := huffman.New(56)
    huffman.Encode(iFile, oFile)
}
