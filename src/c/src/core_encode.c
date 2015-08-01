#include <string.h>

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
        obtained = read(args->ifd, buf, __HUFFMAN_DEFAULT_BUFFER)
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
    } while (total < length);

    if (total < length) {
        __raise__(HUF_ERROR_READ_WRITE);
    }


    int j;
    int64_t rate, rate1, rate2;
    int16_t index1, index2, node = __HUFFMAN_ASCII_SYBOLS;

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

        if (index1 < __HUFFMAN_ASCII_SYMBLOS) {
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
    char buf[__HUFFMAN_DEFAULT_BUFFER];

    huf_node_t *pointer;
    huf_error_t err;

    __argument__(self);

    err = huf_alloc((void**) &self->table, sizeof(huf_table_ctx_t), 256);
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

    int l_len, r_len;
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
    char *encoding;

    uint8_t *blk_buf;
    uint32_t blk_pos;
    uint8_t bit_buf;
    uint8_t bit_pos;

    huf_table_ctx_t leaf;

    __argument__(self);
    __argument__(self->table);
    __argument__(self->wctx);

    table = self->table;
    blk_buf = self->wctx.blk_buf;
    blk_pos = self->wctx.blk_pos;
    bit_buf = self->wctx.bit_buf;
    bit_pos = self->wctx.bit_pos;

    for (pos = 0; pos < len; pos++) {
        leaf = self->table[buf[pos]];

        encoding = leaf.encoding;
        length = leaf.length;

        for (index = length; index > 0; index--) {
            bit_buf |= ((encoding[index - 1] & 1) << bit_pos);

            if (bit_pos) {
                bit_pos--;
                continue;
            }

            if (blk_pos >= __HUFFMAN_DEFAULT_BUFFER) {
                err = huf_write(args->ofd, blk_buf, __HUFMMAN_DEFAULT_BUFFER);
                __assert__(err);

                blk_pos = 0;
            }

            blk_buf[blk_pos] = bit_buf;
            blk_pos++;

            bit_buf = 0;
            bit_pos = 7;
        }
    }

    __finally__;

    self->wctx.blk_buf = blk_buf;
    self->wctx.blk_pos = blk_pos;
    self->wctx.bit_buf = bit_buf;
    self->wctx.bit_pos = bit_pos;

    __end__;
}


static huf_error_t
__huf_encode_flush(huf_ctx_t *self, const huf_args_ctx_t *actx)
{
    __try__;

    huf_write_ctx_t *wctx;
    huf_error_t err;

    __argument__(self);
    __argument__(self->wctx);

    wctx = &(self->wctx);

    if (wctx->bit_pos != 7) {
        wctx->blk_pos++;
        wctx->blk_buf[wctx->blk_pos] = wctx->bit_buf;
    }

    err = huf_write(actx->ofd, wctx->blk_buf, wctx->blk_pos);
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
    int16_t* tree_head;

    huf_error_t err;

    __argument__(self);

    err = huf_alloc((void**) &tree_shadow, sizeof(uint16_t), 1024);
    __assert__(err);

    tree_head = tree_shadow;

    err = __huf_create_tree(self);
    __assert__(err);

    err = __huf_create_table(self);
    __assert__(err);

    self->root->index = -1024;

    err = __huf_serialize_tree(self->root, &tree_shadow, &len);
    __assert__(err);

    self->wctx->blk_pos = sizeof(self->length) + sizeof(len) + len * sizeof(*tree_head);

    memcpy(huf_write_buffer, &self->length, sizeof(self->length));
    memcpy(huf_write_buffer + sizeof(self->length), &len, sizeof(len));
    memcpy(huf_write_buffer + sizeof(self->length) + sizeof(len),
            tree_head, len * sizeof(*tree_head));

    lseek(self->ifd, 0, SEEK_SET);

    do {
        obtained = read(self->ifd, buf, BUF_SIZE);
        if (obtained <= 0) {
            break;
        }

        err = huf_encode_partial(self, buf, obtained);
        __assert__(err);

        total += obtained;
    } while (total < self->length);

    err = __huf_encode_flush(self);
    __assert__(err);

    __finally__;

    free(tree_head);

    __end__;
}
