package huffman

import (
    "io"
)

type huffmanNode struct {
    parent *huffmanNode
    left *huffmanNode
    right *huffmanNode
}

type Huffman struct {
    length uint64
    encoding *[]string
    root *huffmanNode
    leaves *[][]huffmanNode
}

func New(length uint64) *Huffman {
    return &Huffman{length: length}
}

func (context *Huffman) Encode(writer *io.Writer) {
}

func (context *Huffman) Decode(reader *io.Reader) {
}

