#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "huffman/encoder.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


static huf_error_t
__huf_create_tree(huf_encoder_t *self, const huf_read_writer_t *read_writer)
{
    __try__;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];
    int64_t rates[512]; // Frequency histogram array;

    huf_node_t **shadow_tree;
    huf_error_t err;

    __argument__(self);
    __argument__(read_writer);

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
            self->huffman_tree->root = shadow_tree[node-1];
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
            self->huffman_tree->leaves[index1] = shadow_tree[index1];
        }

        if (index2 < __HUFFMAN_ASCII_SYMBOLS) {
            self->huffman_tree->leaves[index2] = shadow_tree[index2];
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

    __end__;
}


static huf_error_t
__huf_create_char_coding(huf_encoder_t *self)
{
    __try__;

    huf_node_t *pointer;
    huf_error_t err;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER] = {0};
    uint8_t *char_encoding = NULL;

    int index, position;

    __argument__(self);

    err = huf_malloc((void**) &self->char_coding, sizeof(huf_char_coding_t), 256);
    __assert__(err);

    for (index = 0; index < 256; index++) {
        pointer = self->huffman_tree->leaves[index];
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
            err = huf_malloc((void**) &char_encoding, sizeof(uint8_t), position + 1);
            __assert__(err);

            memcpy(char_encoding, buf, position);

            self->char_coding[index].length = position;
            self->char_coding[index].encoding = char_encoding;
            printf("%d\t%s\n", index, self->char_coding[index].encoding);
        }
    }

    __finally__;
    __end__;
}

static huf_error_t
__huf_encode_partial(huf_encoder_t* self, const huf_read_writer_t *read_writer, uint8_t *buf, uint64_t len)
{
    __try__;

    uint64_t pos;
    int length, index, err;
    uint8_t *encoding;

    uint8_t bit_rwbuf = 0;
    uint8_t bit_offset = 0;

    huf_char_coding_t leaf;

    __argument__(self);
    __argument__(self->char_coding);
    __argument__(read_writer);

    bit_rwbuf = self->bufio_read_writer->bit_rwbuf;
    bit_offset = self->bufio_read_writer->bit_offset;

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

            err = huf_bufio_write_uint8(self->bufio_read_writer, bit_rwbuf);
            __assert__(err);

            bit_rwbuf = 0;
            bit_offset = 7;
        }
    }

    __finally__;

    self->bufio_read_writer->bit_rwbuf = bit_rwbuf;
    self->bufio_read_writer->bit_offset = bit_offset;

    __end__;
}


static huf_error_t
__huf_encode_flush(huf_encoder_t *self, const huf_read_writer_t *read_writer)
{
    __try__;

    huf_bufio_read_writer_t *bufio_read_writer;
    huf_error_t err;

    __argument__(self);
    __argument__(read_writer);

    bufio_read_writer = self->bufio_read_writer;

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
huf_encoder_init(huf_encoder_t **self, huf_read_writer_t *read_writer)
{
    __try__;

    huf_error_t err;
    huf_encoder_t *self_ptr = NULL;

    __argument__(self);
    __argument__(read_writer);

    err = huf_malloc((void**) &self_ptr, sizeof(huf_encoder_t), 1);
    __assert__(err);

    *self = self_ptr;

    // Allocate memory for Huffman tree.
    err = huf_tree_init(&self_ptr->huffman_tree);
    __assert__(err);

    __debug__("Encoder alloc %p\n", (void*)self_ptr->huffman_tree);

    // Create buffered read-writer instance with 64KiB buffer.
    err = huf_bufio_read_writer_init(&self_ptr->bufio_read_writer, read_writer, __HUFFMAN_DEFAULT_BUFFER);
    __assert__(err);

    __finally__;
    __end__;
}


huf_error_t
huf_encoder_free(huf_encoder_t **self)
{
    __try__;

    huf_error_t err;
    huf_encoder_t *self_ptr;

    __argument__(self);

    self_ptr = *self;

    err = huf_tree_free(&self_ptr->huffman_tree);
    __assert__(err);

    err = huf_bufio_read_writer_free(&self_ptr->bufio_read_writer);
    __assert__(err);

    free(self_ptr);

    *self = NULL;

    __finally__;
    __end__;
}


huf_error_t
huf_encode(huf_reader_t reader, huf_writer_t writer, uint64_t len)
{
    __try__;

    huf_error_t err;
    huf_read_writer_t read_writer = {reader, writer, len};
    huf_encoder_t *self = NULL;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER] = {0};

    int16_t tree_head[__HUFFMAN_MAX_TREE_LENGTH] = {0};
    int16_t actual_tree_length = 0;
    size_t tree_length = 0;

    err = huf_encoder_init(&self, &read_writer);
    __assert__(err);

    err = __huf_create_tree(self, &read_writer);
    __assert__(err);

    err = __huf_create_char_coding(self);
    __assert__(err);

    printf("ROOT %d\n", self->huffman_tree->root->index);
    /*self->huffman_tree->root->index = -1024;*/

    // Write serialized tree into buffer.
    err = huf_tree_serialize(self->huffman_tree, tree_head, &tree_length);
    __assert__(err);

    actual_tree_length = tree_length;

    err = huf_bufio_write(self->bufio_read_writer, &len, sizeof(uint64_t));
    __assert__(err);

    err = huf_bufio_write(self->bufio_read_writer, &actual_tree_length, sizeof(int16_t));
    __assert__(err);

    err = huf_bufio_write(self->bufio_read_writer, tree_head, tree_length * sizeof(int16_t));
    __assert__(err);

    lseek(reader, 0, SEEK_SET);

    size_t left_to_read = read_writer.len;
    size_t need_to_read;

    do {
        need_to_read = self->bufio_read_writer->size;

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

    huf_encoder_free(&self);

    __end__;
}
