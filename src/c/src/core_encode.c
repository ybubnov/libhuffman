#include "rt.h"
#include "core.h"
#include "internal.h"

#include <string.h>


static huf_error_t
__huf_create_tree(huf_ctx_t *hctx, const huf_args_t *args)
{
    __TRY__;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];
    int64_t *rates; // Frequency histogram array;

    huf_node_t **shadow_tree;

    __ASSERT_NULL__(hctx, HUF_ERROR_INVALID_ARGUMENT);
    __ASSERT_NULL__(args, HUF_ERROR_INVALID_ARGUMENT);

    // Read symblos from file and build frequency histogram.
    rates = calloc(512, sizeof(int64_t));

    // Generate an error, if memory was not allocated.
    __ASSERT_NULL__(rates, HUF_ERROR_MEMORY_ALLOCATION);

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


    int j;
    int64_t rate, rate1, rate2;
    int16_t index1, index2, node = __HUFFMAN_ASCII_SYBOLS;

    shadow_tree = calloc(512, sizeof(huf_node_t*));
    __ASSERT_NULL__(shadow_tree, HUF_ERROR_MEMORY_ALLOCATION);

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
            hctx->root = shadow_tree[node-1];
            break;
        }

        if (!shadow_tree[index1]) {
            shadow_tree[index1] = calloc(1, sizeof(huf_node_t));
            __ASSERT_NULL__(shadow_tree[index1], HUF_ERROR_MEMORY_ALLOCATION);
        }

        if (!shadow_tree[index2]) {
            shadow_tree[index2] = calloc(1, sizeof(huf_node_t));
            __ASSERT_NULL__(shadow_tree[index2], HUF_ERROR_MEMORY_ALLOCATION);
        }

        shadow_tree[node] = calloc(1, sizeof(huf_node_t));
        __ASSERT_NULL__(shadow_tree[node], HUF_ERROR_MEMORY_ALLOCATION);

        if (index1 < __HUFFMAN_ASCII_SYMBLOS) {
            hctx->leaves[index1] = shadow_tree[index1];
        }

        if (index2 < __HUFFMAN_ASCII_SYMBOLS) {
            hctx->leaves[index2] = shadow_tree[index2];
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


    __FINALLY__;

    if (__RAISED__) {
        for (j = 0; j < 512; j++) {
            free(shadow_tree[j]);
        }
    }

    free(shadow_tree);
    free(rates);

    __END__;
}


static huf_error_t
__huf_create_table(huf_ctx_t *hctx)
{
    __TRY__;

    int index, position;
    char buf[__HUFFMAN_DEFAULT_BUFFER];

    huf_node_t *pointer;

    __ASSERT_NULL__(hctx, HUF_ERROR_INVALID_ARGUMENT);

    hctx->table = calloc(256, sizeof(huf_table_ctx_t));

    __ASSERT_NULL__(hctx->table, HUF_ERROR_MEMORY_ALLOCATION);

    for (index = 0; index < 256; index++) {
        pointer = hctx->leaves[index];
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
            hctx->table[index].encoding = calloc(position + 1, sizeof(char));
            __ASSERT_NULL__(hctx->table[index].encoding, HUF_MEMORY_ALLOCATION);

            hctx->table[index].length = position;
            memcpy(hctx->table[index].encoding, buf, position);
        }
    }

    __FINALLY__;
    __END__;
}


static huf_error_t
__huf_serialize_tree(huf_node_t* tree, int16_t** dest, int16_t *pos)
{
    __TRY__;

    int l_len, r_len;
    huf_error_t err;

    __ASSERT_NULL__(pos, HUF_ERROR_INVALID_ARGUMENT);
    __ASSERT_NULL__(dest, HUF_ERROR_INVALID_ARGUMENT);

    if (tree) {
        **dest = tree->index;

        (*dest)++;
        err = __huf_serialize_tree(tree->left, dest, &l_len);
        __ASSERT__(err, err);

        (*dest)++;
        err = __huf_serialize_tree(tree->right, dest, &r_len);
        __ASSERT__(err, err);

        *pos = l_len + r_len + 1;
    } else {
        **dest = __HUFFMAN_LEAF;
        *pos = 1;
    }

    __FINALLY__;
    __END__;
}

static huf_error_t
__huf_encode_partial(huf_ctx_t* hctx, const huf_args_t *args, uint8_t *buf, uint64_t len)
{
    __TRY__;

    uint64_t pos;
    int length, index, err;
    char *encoding;

    uint8_t *blk_buf;
    uint32_t blk_pos;
    uint8_t bit_buf;
    uint8_t bit_pos;

    huf_table_ctx_t leaf;

    __ASSERT_NULL__(hctx, HUF_ERROR_INVALID_ARGUMENT);
    __ASSERT_NULL__(hctx->table, HUF_ERROR_INVALID_ARGUMENT);
    __ASSERT_NULL__(hctx->wctx, HUF_ERROR_INVALID_ARGUMENT);

    table = hctx->table;
    blk_buf = hctx->wctx.blk_buf;
    blk_pos = hctx->wctx.blk_pos;
    bit_buf = hctx->wctx.bit_buf;
    bit_pos = hctx->wctx.bit_pos;

    for (pos = 0; pos < len; pos++) {
        leaf = hctx->table[buf[pos]];

        encoding = leaf.encoding;
        length = leaf.length;

        for (index = length; index > 0; index--) {
            bit_buf |= ((encoding[index - 1] & 1) << bit_pos);

            if (bit_pos) {
                bit_pos--;
                continue;
            }

            if (blk_pos >= __HUFFMAN_DEFAULT_BUFFER) {
                err = write(args->ofd, blk_buf, __HUFFMAN_DEFAULT_BUFFER);
                __ASSERT_TRUE__(err == -1, HUF_ERROR_READ_WRITE);

                blk_pos = 0;
            }

            blk_buf[blk_pos] = bit_buf;
            blk_pos++;

            bit_buf = 0;
            bit_pos = 7;
        }
    }

    __FINALLY__;

    hctx->wctx.blk_buf = blk_buf;
    hctx->wctx.blk_pos = blk_pos;
    hctx->wctx.bit_buf = bit_buf;
    hctx->wctx.bit_pos = bit_pos;

    __END__;
}


static huf_error_t
__huf_encode_flush(huf_ctx_t *hctx, const huf_args_ctx_t *actx)
{
    __TRY__;

    huf_write_ctx_t *wctx;

    __ASSERT_NULL__(hctx, HUF_ERROR_INVALID_ARGUMENT);
    __ASSERT_NULL__(hctx->wctx, HUF_ERROR_INVALID_ARGUMENT);

    wctx = &(hctx->wctx);

    if (wctx->bit_pos != 7) {
        wctx->blk_pos++;
        wctx->blk_buf[wctx->blk_pos] = wctx->bit_buf;
    }

    return write(actx->ofd, wctx->blk_buf, wctx->blk_pos);

    __FINALLY__;
    __END__;
}


huf_error_t
huf_encode(huf_ctx_t* hctx, int ifd, int ofd, uint64_t len)
{
    __TRY__;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];
    uint64_t obtained, total = 0;

    int16_t* tree_shadow;
    int16_t* tree_head;

    huf_error_t err;

    tree_shadow = calloc(1024, sizeof(uint16_t));
    __ASSERT_NULL__(tree_shadow, HUF_ERROR_MEMORY_ALLOCATION);

    tree_head = tree_shadow;

    err = __huf_create_tree(hctx);
    __ASSERT__(err, err);

    err = __huf_create_table(hctx);
    __ASSERT__(err, err);

    hctx->root->index = -1024;
    int16_t len = __huf_serialize_tree(hctx->root, &tree_shadow);

    huf_write_pos = sizeof(hctx->length) + sizeof(len) + len * sizeof(*tree_head);

    memcpy(huf_write_buffer, &hctx->length, sizeof(hctx->length));
    memcpy(huf_write_buffer + sizeof(hctx->length), &len, sizeof(len));
    memcpy(huf_write_buffer + sizeof(hctx->length) + sizeof(len),
            tree_head, len * sizeof(*tree_head));

    lseek(hctx->ifd, 0, SEEK_SET);

    do {
        obtained = read(hctx->ifd, buf, BUF_SIZE);
        if (obtained <= 0) {
            break;
        }

        err = huf_encode_partial(hctx, buf, obtained);
        __ASSERT__(err);

        total += obtained;
    } while(total < hctx->length);

    err = __huf_encode_flush(hctx);
    __ASSERT__(err, err);

    __FINALLY__;

    free(tree_head);

    __END__;
}
