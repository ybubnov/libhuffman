package huffman

import (
    "io"
    "fmt"
    "bufio"
)


type huffmanNode struct {
    index int16
    parent *huffmanNode
    left *huffmanNode
    right *huffmanNode
}

type Huffman struct {
    length uint64
    encoding *map[byte]string
    root *huffmanNode
    leaves *[][]huffmanNode
}

func New(length uint64) *Huffman {
    return &Huffman{length: length}
}

func (self *Huffman) Encode(reader io.Reader) error {
    if err := self.makeTree(reader); err != nil {
        return err
    }

    return nil
}

func (self *Huffman) Decode(writer io.Writer) error {
    return nil
}


func (self *Huffman) makeTree(reader io.Reader) error {
    rates, obtained, start := make([]int16, 512), uint64(0), 0
    bReader := bufio.NewReader(reader)

    for obtained < self.length {
        symbol, err := bReader.ReadByte()
        if err == nil {
            index := int(symbol)
            rates[index] += 1
            obtained += 1

            if (index < start) {
                start = index
            }
        } else if err == io.EOF {
            break
        } else {
            return err
        }
    }

    fmt.Printf("RATES->%v\n", rates)
    return nil
}

