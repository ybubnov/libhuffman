#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "runtime.h"
#include "core.h"
#include "internal.h"


static huf_error_t
__huf_create_tree(huf_ctx_t *self, const huf_args_t *args)
{
    __try__;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];
    int64_t *rates; // Frequency histogram array;

    huf_node_t **shadow_tree;
    huf_error_t err;

    __argument__(self);
    __argument__(args);

    // Read symblos from file and build frequency histogram.
    err = huf_alloc((void**) &rates, sizeof(int64_t), 512);
    // Generate an error, if memory was not allocated.
    __assert__(err);

    int index, start = __HUFFMAN_ASCII_SYMBOLS;
    uint64_t i, total = 0, obtained = 0;

    do {
        obtained = read(args->ifd, buf, __HUFFMAN_DEFAULT_BUFFER);
        if (obtained <= 0) {
            break;
        }

        // Calculate frequency of the symbols.
        for (i = 0; i < obtained; i++) {
            index = buf[i];
            rates[index]++;

            if (index < start) {
                start = index;
            }
        }

        total += obtained;
    } while (total < args->len);

    if (total < args->len) {
        __raise__(HUF_ERROR_READ_WRITE);
    }


    int j;
    int64_t rate, rate1, rate2;
    int16_t index1, index2, node = __HUFFMAN_ASCII_SYMBOLS;

    err = huf_alloc((void**) &shadow_tree, sizeof(huf_node_t*), 512);
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
            err = huf_alloc((void**) &shadow_tree[index1], sizeof(huf_node_t), 1);
            __assert__(err);
        }

        if (!shadow_tree[index2]) {
            err = huf_alloc((void**) &shadow_tree[index2], sizeof(huf_node_t), 1);
            __assert__(err);
        }

        err = huf_alloc((void**) &shadow_tree[node], sizeof(huf_node_t), 1);
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
__huf_create_table(huf_ctx_t *self)
{
    __try__;

    int index, position;
    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];

    huf_node_t *pointer;
    huf_error_t err;

    __argument__(self);

    err = huf_alloc((void**) &self->table, sizeof(huf_char_coding_t), 256);
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
            err = huf_alloc((void**) &(self->table[index].encoding), sizeof(char), 1);
            __assert__(err);

            self->table[index].length = position;
            memcpy(self->table[index].encoding, buf, position);
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
__huf_encode_partial(huf_ctx_t* self, const huf_args_t *args, uint8_t *buf, uint64_t len)
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
    __argument__(self->table);
    // __argument__(self->read_writer);

    byte_rwbuf = self->read_writer.byte_rwbuf;
    byte_offset = self->read_writer.byte_offset;
    bit_rwbuf = self->read_writer.bit_rwbuf;
    bit_offset = self->read_writer.bit_offset;

    for (pos = 0; pos < len; pos++) {
        leaf = self->table[buf[pos]];

        encoding = leaf.encoding;
        length = leaf.length;

        for (index = length; index > 0; index--) {
            bit_rwbuf |= ((encoding[index - 1] & 1) << bit_offset);

            if (bit_offset) {
                bit_offset--;
                continue;
            }

            if (byte_offset >= __HUFFMAN_DEFAULT_BUFFER) {
                err = huf_write(args->ofd, byte_rwbuf, __HUFFMAN_DEFAULT_BUFFER);
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

    self->read_writer.byte_rwbuf = byte_rwbuf;
    self->read_writer.byte_offset = byte_offset;
    self->read_writer.bit_rwbuf = bit_rwbuf;
    self->read_writer.bit_offset = bit_offset;

    __end__;
}


static huf_error_t
__huf_encode_flush(huf_ctx_t *self, const huf_args_t *actx)
{
    __try__;

    huf_read_writer_t *read_writer;
    huf_error_t err;

    __argument__(self);
    // __argument__(self->read_writer);

    read_writer = &(self->read_writer);

    if (read_writer->bit_offset != 7) {
        read_writer->byte_offset++;
        read_writer->byte_rwbuf[read_writer->byte_offset] = read_writer->bit_rwbuf;
    }

    err = huf_write(actx->ofd, read_writer->byte_rwbuf, read_writer->byte_offset);
    __assert__(err);

    __finally__;
    __end__;
}


huf_error_t
huf_encode(huf_ctx_t* self, int ifd, int ofd, uint64_t len)
{
    __try__;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];
    uint64_t obtained, total = 0;

    int16_t* tree_shadow;
    int16_t* tree_head = 0;
    int16_t length;

    huf_error_t err;
    huf_args_t args = {ifd, ofd, len};

    __argument__(self);

    err = huf_alloc((void**) &tree_shadow, sizeof(uint16_t), 1024);
    __assert__(err);

    tree_head = tree_shadow;

    err = __huf_create_tree(self, &args);
    __assert__(err);

    err = __huf_create_table(self);
    __assert__(err);

    self->root->index = -1024;

    err = __huf_serialize_tree(self->root, &tree_shadow, &length);
    __assert__(err);

    self->read_writer.byte_offset = sizeof(len) + sizeof(length) + length * sizeof(*tree_head);

    memcpy(self->read_writer.byte_rwbuf, &len, sizeof(len));
    memcpy(self->read_writer.byte_rwbuf+ sizeof(len), &length, sizeof(length));
    memcpy(self->read_writer.byte_rwbuf+ sizeof(len) + sizeof(length),
            tree_head, length * sizeof(*tree_head));

    lseek(ifd, 0, SEEK_SET);

    do {
        obtained = read(ifd, buf, __HUFFMAN_DEFAULT_BUFFER);
        if (obtained <= 0) {
            break;
        }

        err = __huf_encode_partial(self, &args, buf, obtained);
        __assert__(err);

        total += obtained;
    } while (total < len);

    err = __huf_encode_flush(self, &args);
    __assert__(err);

    __finally__;

    free(tree_head);

    __end__;
}
