package huffman

import (
    "io"
    "fmt"
    "bufio"
    "bytes"
)

const (
    BUFFER_SIZE int = 65536
    LEAF int = 1024
)


type huffmanNode struct {
    index int
    parent *huffmanNode
    left *huffmanNode
    right *huffmanNode
}

type Huffman struct {
    length uint64
    encoding *map[int][]byte
    root *huffmanNode
    leaves []*huffmanNode
}

func New(length uint64) *Huffman {
    leaves := make([]*huffmanNode, 256)
    return &Huffman{length: length, leaves: leaves}
}

func (self *Huffman) Encode(reader io.Reader, writer io.Writer) error {
    if err := self.buildEncodingTree(reader); err != nil {
        return err
    }

    if err := self.buildEncodingMap(); err != nil {
        return err
    }

    buffer := make([]byte, 1024)
    actualLen := self.serializeTree(self.root, buffer)
    fmt.Printf("SERIALIZED TREE: %v", buffer[:actualLen])
    writer.Write(buffer[:actualLen])

    return nil
}

func (self *Huffman) Decode(reader io.Reader, writer io.Writer) error {
    return nil
}

func (self *Huffman) buildEncodingTree(reader io.Reader) error {
    rates, total, start := make([]uint16, 512), uint64(0), 0
    bReader, buf := bufio.NewReader(reader), make([]byte, BUFFER_SIZE)

    for total < self.length {
        if obtained, err := bReader.Read(buf); err == nil {
            total += uint64(obtained)
            for _, symbol := range buf[:obtained]{
                index := int(symbol)
                rates[index]++
                obtained++

                if (index < start) {
                    start = index
                }
            }
        } else if err == io.EOF {
            break
        } else {
            return err
        }
    }

    index1, index2, node := 0, 0, 256
    var rate1, rate2 uint16
    flatTree := make([]*huffmanNode, 512)

    for start < 512 {
        index1, index2 = -1, -1
        rate1, rate2 = 0, 0

        for _, value := range rates[start:] {
            if value == 0 {
                start++
            } else {
                break
            }
        }

        for position, rate := range rates[start:node] {
            if rate != 0 {
                position += start
                if rate1 == 0 {
                    index1, rate1 = position, rate
                } else if rate < rate1 {
                    index1, index2 = position, index1
                    rate1, rate2 = rate, rate1
                } else if rate2 == 0 {
                    index2, rate2 = position, rate
                } else if rate < rate2 {
                    index2, rate2 = position, rate
                }
            }
        }

        if index1 == -1 || index2 == -1 {
            self.root = flatTree[node-1]
            break
        }

        if flatTree[index1] == nil {
            flatTree[index1] = &huffmanNode{index: index1}
        }

        if flatTree[index2] == nil {
            flatTree[index2] = &huffmanNode{index: index2}
        }

        if index1 < 256 {
            self.leaves[index1] = flatTree[index1]
        }

        if index2 < 256 {
            self.leaves[index2] = flatTree[index2]
        }

        flatTree[node] = &huffmanNode{
            index: node,
            left: flatTree[index1],
            right: flatTree[index2]}

        flatTree[index1].parent = flatTree[node]
        flatTree[index2].parent = flatTree[node]
        flatTree[node].left = flatTree[index1]
        flatTree[node].right = flatTree[index2]

        rates[node] = rate1 + rate2
        rates[index1], rates[index2] = 0, 0
        node++
    }

    fmt.Printf("RATES: %v\n", rates)
    fmt.Printf("HUFFMAN: %v\n", self)
    return nil
}

func (self *Huffman) buildDecodingTree(writer io.Writer) error {
    return nil
}

func (self *Huffman) buildEncodingMap() error {
    var buffer bytes.Buffer
    encoding := map[int][]byte{}

    for position, pointer := range self.leaves {
        for pointer != nil {
            if pointer.parent != nil {
                if pointer.parent.left == pointer {
                    buffer.WriteByte(0)
                } else if pointer.parent.right == pointer {
                    buffer.WriteByte(1)
                }
            }

            pointer = pointer.parent
        }

        if self.leaves[position] != nil {
            bytes := buffer.Bytes()
            encoding[position] = make([]byte, len(bytes))
            copy(encoding[position], bytes)
            buffer.Reset()
        }
    }

    fmt.Printf("ENCODING: %v\n", encoding)
    self.encoding = &encoding
    return nil
}

func (self *Huffman) serializeTree(node *huffmanNode, buffer []byte) int {
    if (node != nil) {
        buffer[0], buffer[1] = byte(node.index >> 0x8), byte(node.index & 0xff)
        leftLen := self.serializeTree(node.left, buffer[2:])
        rightLen := self.serializeTree(node.right, buffer[leftLen+2:])
        return leftLen + rightLen + 2
    }

    buffer[0], buffer[1] = byte(LEAF >> 0x8), byte(LEAF & 0xff)
    return 2
}

func (self *Huffman) deserializeTree(reader io.Reader) error {
    return nil
}

