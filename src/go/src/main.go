package main

import (
    "os"
    "log"
    "huffman"
)

func main() {
    iFile, err := os.OpenFile(
                    "/home/kubrick/test/file.txt",
                    os.O_RDONLY, 0666);
    if err != nil {
        log.Fatal(err)
    }

    defer iFile.Close()


    oFile, err := os.Create("/home/kubrick/test/out.txt")
    if err != nil {
        log.Fatal(err)
    }

    defer oFile.Close()

    dFile, err := os.Create("/home/kubrick/test/decoded.txt")
    if err != nil {
        log.Fatal(err)
    }
    defer dFile.Close()

    fileInfo, err := iFile.Stat()
    if err != nil {
        log.Fatal(err)
    }

    huffman := huffman.New(fileInfo.Size())
    huffman.Encode(iFile, oFile)

    oFile.Seek(0, os.SEEK_SET)
    huffman.Decode(oFile, dFile)
}

