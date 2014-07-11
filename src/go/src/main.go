package main

import (
    "os"
    "log"
    "huffman"
)

func main() {
    file, err := os.Open("/home/kubrick/test/file.txt")
    defer file.Close()

    if err != nil {
        log.Fatal(err)
    }

    huffman := huffman.New(56)
    huffman.Encode(file)
}
