#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <huffman/huffman.h>
#include <huffman/runtime/malloc.h>
#include <huffman/runtime/sys.h>


static huf_error_t
__huf_create_tree(huf_archiver_t *self, const huf_read_writer_t *read_writer)
{
    __try__;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];
    int64_t *rates; // Frequency histogram array;

    huf_node_t **shadow_tree;
    huf_error_t err;

    __argument__(self);
    __argument__(read_writer);

    // Read symblos from file and build frequency histogram.
    err = huf_malloc((void**) &rates, sizeof(int64_t), 512);
    // Generate an error, if memory was not allocated.
    __assert__(err);

    int i, index, start = __HUFFMAN_ASCII_SYMBOLS;

    size_t left_to_read = read_writer->len;
    size_t need_to_read;

    do {
        // Set as default count of bytes to read.
        need_to_read = __HUFFMAN_DEFAULT_BUFFER;

        if (left_to_read - need_to_read < 0) {
            need_to_read = left_to_read;
        }

        err = huf_read(read_writer->reader, buf, &need_to_read);
        __assert__(err);

        // Calculate frequency of the symbols.
        for (i = 0; i < need_to_read; i++) {
            index = buf[i];
            rates[index]++;

            if (index < start) {
                start = index;
            }
        }

        left_to_read -= need_to_read;
    } while (left_to_read);


    int j;
    int64_t rate, rate1, rate2;
    int16_t index1, index2, node = __HUFFMAN_ASCII_SYMBOLS;

    err = huf_malloc((void**) &shadow_tree, sizeof(huf_node_t*), 512);
    __assert__(err);

    while (start < 512) {
        index1 = index2 = -1;
        rate1 = rate2 = 0;

        while (!rates[start]) {
            start++;
        }

        for (j = start; j < node; j++) {
            rate = rates[j];

            if (rate) {
                if (!rate1) {
                    rate1 = rate;
                    index1 = j;
                } else if (rate <= rate1) {
                    rate2 = rate1;
                    rate1 = rate;
                    index2 = index1;
                    index1 = j;
                } else if (!rate2 || rate <= rate2) {
                    rate2 = rate;
                    index2 = j;
                }
            }
        }

        // Tree is constructed.
        if (index1 == -1 || index2 == -1) {
            self->root = shadow_tree[node-1];
            break;
        }

        if (!shadow_tree[index1]) {
            err = huf_malloc((void**) &shadow_tree[index1], sizeof(huf_node_t), 1);
            __assert__(err);
        }

        if (!shadow_tree[index2]) {
            err = huf_malloc((void**) &shadow_tree[index2], sizeof(huf_node_t), 1);
            __assert__(err);
        }

        err = huf_malloc((void**) &shadow_tree[node], sizeof(huf_node_t), 1);
        __assert__(err);

        if (index1 < __HUFFMAN_ASCII_SYMBOLS) {
            self->leaves[index1] = shadow_tree[index1];
        }

        if (index2 < __HUFFMAN_ASCII_SYMBOLS) {
            self->leaves[index2] = shadow_tree[index2];
        }

        shadow_tree[index1]->parent = shadow_tree[node];
        shadow_tree[index2]->parent = shadow_tree[node];
        shadow_tree[node]->left = shadow_tree[index1];
        shadow_tree[node]->right = shadow_tree[index2];

        shadow_tree[index1]->index = index1;
        shadow_tree[index2]->index = index2;
        shadow_tree[node]->index = node;

        rates[node] = rate1 + rate2;
        rates[index1] = 0;
        rates[index2] = 0;
        node++;
    }


    __finally__;

    if (__raised__) {
        for (j = 0; j < 512; j++) {
            free(shadow_tree[j]);
        }
    }

    free(shadow_tree);
    free(rates);

    __end__;
}


static huf_error_t
__huf_create_char_coding(huf_archiver_t *self)
{
    __try__;

    int index, position;
    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];

    huf_node_t *pointer;
    huf_error_t err;

    __argument__(self);

    err = huf_malloc((void**) &self->char_coding, sizeof(huf_char_coding_t), 256);
    __assert__(err);

    for (index = 0; index < 256; index++) {
        pointer = self->leaves[index];
        position = 0;

        while (pointer) {
            if (pointer->parent) {
                if (pointer->parent->left == pointer) {
                    buf[position] = '0';
                } else if (pointer->parent->right == pointer) {
                    buf[position] = '1';
                }

                position++;
            }

            pointer = pointer->parent;
        }

        if (position) {
            err = huf_malloc((void**) &(self->char_coding[index].encoding), sizeof(char), 1);
            __assert__(err);

            self->char_coding[index].length = position;
            memcpy(self->char_coding[index].encoding, buf, position);
        }
    }

    __finally__;
    __end__;
}


static huf_error_t
__huf_serialize_tree(huf_node_t* tree, int16_t** dest, int16_t *pos)
{
    __try__;

    int16_t l_len, r_len;
    huf_error_t err;

    __argument__(pos);
    __argument__(dest);

    if (tree) {
        **dest = tree->index;

        (*dest)++;
        err = __huf_serialize_tree(tree->left, dest, &l_len);
        __assert__(err);

        (*dest)++;
        err = __huf_serialize_tree(tree->right, dest, &r_len);
        __assert__(err);

        *pos = l_len + r_len + 1;
    } else {
        **dest = __HUFFMAN_LEAF;
        *pos = 1;
    }

    __finally__;
    __end__;
}

static huf_error_t
__huf_encode_partial(huf_archiver_t* self, const huf_read_writer_t *read_writer, uint8_t *buf, uint64_t len)
{
    __try__;

    uint64_t pos;
    int length, index, err;
    uint8_t *encoding;

    uint8_t *byte_rwbuf = 0;
    uint32_t byte_offset = 0;
    uint8_t bit_rwbuf = 0;
    uint8_t bit_offset = 0;

    huf_char_coding_t leaf;

    __argument__(self);
    __argument__(self->char_coding);
    // __argument__(self->bufio_read_writer);

    byte_rwbuf = self->bufio_read_writer.byte_rwbuf;
    byte_offset = self->bufio_read_writer.byte_offset;
    bit_rwbuf = self->bufio_read_writer.bit_rwbuf;
    bit_offset = self->bufio_read_writer.bit_offset;

    for (pos = 0; pos < len; pos++) {
        leaf = self->char_coding[buf[pos]];

        encoding = leaf.encoding;
        length = leaf.length;

        for (index = length; index > 0; index--) {
            bit_rwbuf |= ((encoding[index - 1] & 1) << bit_offset);

            if (bit_offset) {
                bit_offset--;
                continue;
            }

            if (byte_offset >= __HUFFMAN_DEFAULT_BUFFER) {
                err = huf_write(read_writer->writer, byte_rwbuf, __HUFFMAN_DEFAULT_BUFFER);
                __assert__(err);

                byte_offset = 0;
            }

            byte_rwbuf[byte_offset] = bit_rwbuf;
            byte_offset++;

            bit_rwbuf = 0;
            bit_offset = 7;
        }
    }

    __finally__;

    self->bufio_read_writer.byte_rwbuf = byte_rwbuf;
    self->bufio_read_writer.byte_offset = byte_offset;
    self->bufio_read_writer.bit_rwbuf = bit_rwbuf;
    self->bufio_read_writer.bit_offset = bit_offset;

    __end__;
}


static huf_error_t
__huf_encode_flush(huf_archiver_t *self, const huf_read_writer_t *read_writer)
{
    __try__;

    huf_bufio_read_writer_t *bufio_read_writer;
    huf_error_t err;

    __argument__(self);
    // __argument__(self->bufio_read_writer);

    bufio_read_writer = &(self->bufio_read_writer);

    if (bufio_read_writer->bit_offset != 7) {
        bufio_read_writer->byte_offset++;
        bufio_read_writer->byte_rwbuf[bufio_read_writer->byte_offset] = bufio_read_writer->bit_rwbuf;
    }

    err = huf_write(read_writer->writer, bufio_read_writer->byte_rwbuf, bufio_read_writer->byte_offset);
    __assert__(err);

    __finally__;
    __end__;
}


huf_error_t
huf_encode(huf_archiver_t* self, huf_reader_t reader, huf_writer_t writer, uint64_t len)
{
    __try__;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];

    int16_t* tree_shadow;
    int16_t* tree_head = 0;
    int16_t length;

    huf_error_t err;
    huf_read_writer_t read_writer = {reader, writer, len};

    __argument__(self);

    err = huf_malloc((void**) &tree_shadow, sizeof(uint16_t), 1024);
    __assert__(err);

    tree_head = tree_shadow;

    err = __huf_create_tree(self, &read_writer);
    __assert__(err);

    err = __huf_create_char_coding(self);
    __assert__(err);

    self->root->index = -1024;

    err = __huf_serialize_tree(self->root, &tree_shadow, &length);
    __assert__(err);

    self->bufio_read_writer.byte_offset = sizeof(len) + sizeof(length) + length * sizeof(*tree_head);

    memcpy(self->bufio_read_writer.byte_rwbuf, &len, sizeof(len));
    memcpy(self->bufio_read_writer.byte_rwbuf+ sizeof(len), &length, sizeof(length));
    memcpy(self->bufio_read_writer.byte_rwbuf+ sizeof(len) + sizeof(length),
            tree_head, length * sizeof(*tree_head));

    lseek(reader, 0, SEEK_SET);

    size_t left_to_read = read_writer.len;
    size_t need_to_read;

    do {
        need_to_read = __HUFFMAN_DEFAULT_BUFFER;

        if (left_to_read - need_to_read < 0) {
            need_to_read = left_to_read;
        }

        err = huf_read(reader, buf, &need_to_read);
        __assert__(err);

        err = __huf_encode_partial(self, &read_writer, buf, need_to_read);
        __assert__(err);

        left_to_read -= need_to_read;
    } while (left_to_read);

    err = __huf_encode_flush(self, &read_writer);
    __assert__(err);

    __finally__;

    free(tree_head);

    __end__;
}
