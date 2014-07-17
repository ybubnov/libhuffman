package huffman

import (
    "io"
    "os"
    "fmt"
    "bytes"
    "errors"
    "encoding/binary"
)

const (
    BUFFER_SIZE uint = 65536
    HUFFMAN_TREE_LEAF int16 = 1024
)


type huffmanNode struct {
    index int
    parent *huffmanNode
    left *huffmanNode
    right *huffmanNode
}

type huffmanBuffer struct{
    byteBuf []byte
    bitBuf byte
    byteBufPos uint
    bitBufPos uint
    node *huffmanNode
}

type Huffman struct {
    length int64
    encoding *map[int][]byte
    root *huffmanNode
    leaves []*huffmanNode
}

func New(length int64) *Huffman {
    leaves := make([]*huffmanNode, 256)
    return &Huffman{length: length,leaves: leaves}
}

func (self *Huffman) Encode(reader io.ReadSeeker, writer io.Writer) error {
    var err error
    if err = self.buildEncodingTree(reader); err != nil {
        return err
    }

    if err = self.buildEncodingMap(); err != nil {
        return err
    }

    tree := make([]int16, 1024)
    actualLen, err := self.serializeTree(self.root, tree)

    if err != nil {
        return err
    }

    fmt.Printf("SERIALIZED TREE: %v\n", tree[:actualLen])
    binary.Write(writer, binary.LittleEndian, self.length)
    binary.Write(writer, binary.LittleEndian, int16(actualLen))
    binary.Write(writer, binary.LittleEndian, tree[:actualLen])

    reader.Seek(0, os.SEEK_SET)
    raw, total := make([]byte, BUFFER_SIZE), int64(0)

    buf := &huffmanBuffer{byteBuf: make([]byte, BUFFER_SIZE),
                          bitBufPos: 7}

    for total < self.length {
        if obtained, err := reader.Read(raw); err == nil {
            total += int64(obtained)
            if _, err := self.encodePartial(writer,
                                            raw[:obtained],
                                            buf); err != nil {
                return err
            }
        } else if err == io.EOF {
            break
        } else {
            return err
        }
    }

    if err := self.encodeFlush(writer, buf); err != nil {
        return err
    }

    return nil
}

func (self *Huffman) Decode(reader io.Reader, writer io.Writer) error {
    totalLen, treeLen := int64(0), int16(0)

    if err := binary.Read(reader, binary.LittleEndian, &totalLen); err != nil {
        return err
    }

    if err := binary.Read(reader, binary.LittleEndian, &treeLen); err != nil {
        return nil
    }

    tree := make([]int16, treeLen)
    if err := binary.Read(reader, binary.LittleEndian, &tree); err != nil {
        return nil
    }

    fmt.Printf("TREE: %v\n", tree)
    node, err := self.deserializeTree(tree)
    self.root = node

    if err != nil {
        return err
    }

    raw, total := make([]byte, BUFFER_SIZE), int64(0)
    buf := &huffmanBuffer{byteBuf: make([]byte, BUFFER_SIZE)}

    for total < self.length {
        if obtained, err := reader.Read(raw); err == nil {
            total += int64(obtained)
            if n, err := self.decodePartial(writer,
                                            raw[:obtained],
                                            buf); err != nil {
                return err
            } else {
                totalLen -= int64(n)
            }
        } else if err == io.EOF {
            break
        } else {
            return err
        }
    }

    buf.byteBufPos = uint(totalLen)
    if err := self.decodeFlush(writer, buf); err != nil {
        return err
    }

    return err
}

func (self *Huffman) encodePartial(writer io.Writer,
                                   raw []byte,
                                   buf *huffmanBuffer) (int, error) {
    var written int
    if self.encoding == nil {
        return 0, errors.New("Unexpected error. Invalid encoding table.")
    }

    for _, symbol := range raw {
        encoding := (*self.encoding)[int(symbol)]

        for index := range encoding {
            bit := encoding[len(encoding)-index-1]
            buf.bitBuf |= (bit & 1) << buf.bitBufPos

            if buf.bitBufPos == 0 {
                if buf.byteBufPos >= BUFFER_SIZE {
                    if n, err := writer.Write(buf.byteBuf); err != nil {
                        return 0, err
                    } else {
                        written += n
                    }

                    buf.byteBufPos = 0
                }

                buf.byteBuf[buf.byteBufPos] = buf.bitBuf
                buf.byteBufPos++
                buf.bitBuf = 0
                buf.bitBufPos = 8
            }

            buf.bitBufPos--
        }
    }

    return written, nil
}

func (self *Huffman) decodePartial(writer io.Writer,
                                   raw []byte,
                                   buf *huffmanBuffer) (int, error) {
    written, bit_pos := 0, uint(0)

    if buf.node == nil {
        buf.node = self.root
    }

    for _, symbol := range raw {
        for bit_pos = 8; bit_pos > 0; bit_pos-- {
            if (symbol >> (bit_pos-1)) & 1 == 0 {
                if buf.node = buf.node.left; buf.node == nil {
                    return 0, errors.New("Decoding tree is broken.")
                }
            } else {
                if buf.node = buf.node.right; buf.node == nil {
                    return 0, errors.New("Decoding tree is broken.")
                }
            }

            if buf.node.left == nil && buf.node.right == nil {
                if buf.byteBufPos >= BUFFER_SIZE {
                    if n, err := writer.Write(buf.byteBuf); err != nil {
                        return 0, err
                    } else {
                        written += n
                    }

                    buf.byteBufPos = 0
                }

                buf.byteBuf[buf.byteBufPos] = byte(buf.node.index)
                fmt.Println(buf.node.index)
                buf.byteBufPos++
                buf.node = self.root
            }
        }
    }

    return written, nil
}

func (self *Huffman) encodeFlush(writer io.Writer, buf *huffmanBuffer) error {
    if buf.bitBufPos != 7 {
        buf.byteBuf[buf.byteBufPos] = buf.bitBuf
        buf.byteBufPos++
    }

    _, err := writer.Write(buf.byteBuf[:buf.byteBufPos])
    return err
}

func (self *Huffman) decodeFlush(writer io.Writer, buf *huffmanBuffer) error {
    _, err := writer.Write(buf.byteBuf[:buf.byteBufPos])
    return err
}

func (self *Huffman) buildEncodingTree(reader io.Reader) error {
    rates, total, start := make([]int64, 512), int64(0), 0
    buf := make([]byte, BUFFER_SIZE)

    for total < self.length {
        if obtained, err := reader.Read(buf); err == nil {
            total += int64(obtained)
            for _, symbol := range buf[:obtained]{
                index := int(symbol)
                rates[index]++
                obtained++

                if (index < start) {
                    start = index
                }
            }
        } else if err == io.EOF {
            self.length = total
            break
        } else {
            return err
        }
    }

    index1, index2, node := 0, 0, 256
    var rate1, rate2 int64
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
                } else if rate <= rate1 {
                    index1, index2 = position, index1
                    rate1, rate2 = rate, rate1
                } else if rate2 == 0 || rate <= rate2 {
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

        rates[node] = rate1 + rate2
        rates[index1], rates[index2] = 0, 0
        node++
    }

    fmt.Printf("RATES: %v\n", rates)
    fmt.Printf("HUFFMAN: %v\n", self)
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

func (self *Huffman) serializeTree(node *huffmanNode, buffer []int16) (int, error) {
    if len(buffer) < 1 {
        return 0, errors.New("Unexpected end of buffer")
    }

    if (node != nil) {
        buffer[0] = int16(node.index)

        if leftLen, err := self.serializeTree(node.left, buffer[1:]); err != nil {
            return 0, err
        } else {
            rightLen, err := self.serializeTree(node.right, buffer[leftLen+1:])
            return leftLen + rightLen + 1, err
        }
    }

    buffer[0] = HUFFMAN_TREE_LEAF
    return 1, nil
}

func (self *Huffman) deserializeTree(buffer []int16) (*huffmanNode, error) {
    var deserialize func([]int16) (*huffmanNode, []int16, error)

    deserialize = func(buffer []int16) (*huffmanNode, []int16, error) {
        var err error
        if len(buffer) < 1 {
            return nil, nil, errors.New("Unexpected end of buffer")
        }

        if buffer[0] != HUFFMAN_TREE_LEAF {
            node := &huffmanNode{index: int(buffer[0])}
            if node.left, buffer, err = deserialize(buffer[1:]); err != nil {
                return nil, buffer, err
            }

            if node.right, buffer, err = deserialize(buffer); err != nil {
                return nil, buffer, err
            }

            return node, buffer, nil
        }

        return nil, buffer[1:], nil
    }

    node, _, err := deserialize(buffer)
    return node, err
}

