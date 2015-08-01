#include <unistd.h>

#include "core.h"
#include "internal.h"
#include "runtime.h"


static huf_error_t
__huf_decode_partial(huf_ctx_t *hctx, const huf_args_t *args, uint8_t *buf, uint64_t len, uint64_t *written)
{
    __try__;

    uint64_t pos;
    int err;

    uint8_t *blk_buf;
    uint8_t bit_buf;
    uint8_t bit_pos;
    uint32_t blk_pos;

    __argument__(hctx);
    __argument__(args);
    __argument__(buf);
    __argument__(written);

    blk_buf = hctx->wctx.blk_buf;
    blk_pos = hctx->wctx.blk_pos;
    bit_buf = hctx->wctx.bit_buf;
    bit_pos = hctx->wctx.bit_pos;
    *written = 0;

    if (hctx->last_node == 0) {
        hctx->last_node = hctx->root;
    }

    for (pos = 0; pos < len; pos++) {
        bit_buf = buf[pos];

        for (bit_pos = 8; bit_pos > 0; bit_pos--) {
            if ((bit_buf >> (bit_pos - 1)) & 1) {
                hctx->last_node = hctx->last_node->right;
            } else {
                hctx->last_node = hctx->last_node->left;
            }

            if (hctx->last_node->left || hctx->last_node->right) {
                continue;
            }

            if (blk_pos >= __HUFFMAN_DEFAULT_BUFFER) {
                err = huf_write(args->ofd, blk_buf, __HUFFMAN_DEFAULT_BUFFER);
                __assert__(err);

                *written += __HUFFMAN_DEFAULT_BUFFER;
                blk_pos = 0;
            }

            // put letter into the buffer
            blk_pos++;
            blk_buf[blk_pos] = hctx->last_node->index;

            hctx->last_node = hctx->root;
        }
    }

    __finally__;

    hctx->wctx.blk_buf = blk_buf;
    hctx->wctx.blk_pos = blk_pos;
    hctx->wctx.bit_buf = bit_buf;
    hctx->wctx.bit_pos = bit_pos;

    __end__;
}


static huf_error_t
__huf_decode_flush(const huf_ctx_t *hctx, const huf_args_t *args, int extra)
{
    __try__;

    huf_error_t err;

    __argument__(hctx);
    __argument__(args);

    err = huf_write(args->ofd, hctx->wctx.blk_buf, extra);
    __assert__(err);

    __finally__;
    __end__;
}


static huf_error_t
__huf_deserialize_tree(huf_node_t **node, int16_t **src, int16_t *src_end)
{
    __try__;

    __argument__(node);
    __argument__(src);
    __argument__(src_end);

    int16_t n_index = **src;
    huf_node_t **node_left;
    huf_node_t **node_right;

    huf_error_t err;

    if ((*src) + 1 > src_end) {
        __raise__(HUF_ERROR_INVALID_ARGUMENT);
    }

    (*src)++;

    if (n_index != __HUFFMAN_LEAF) {
        err = huf_alloc((void**) node, sizeof(huf_node_t), 1);
        __assert__(err);

        (*node)->index = n_index;

        node_left = &((*node)->left);
        node_right = &((*node)->right);

        err = __huf_deserialize_tree(node_left, src, src_end);
        __assert__(err);

        err = __huf_deserialize_tree(node_right, src, src_end);
        __assert__(err);
    }

    __finally__;
    __end__;
}


huf_error_t
huf_decode(huf_ctx_t* hctx, int ifd, int ofd, uint64_t len)
{
    __try__;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];
    uint64_t obtained, total = 0;
    uint64_t if_length, written;

    int16_t tree_length = 0;
    int16_t *tree_head, *tree_shadow;

    huf_node_t* root;
    huf_error_t err;
    huf_args_t args = {ifd, ofd, len};

    __argument__(hctx);

    err = huf_read(ifd, &if_length, sizeof(if_length));
    __assert__(err);

    err = huf_read(ifd, &tree_length, sizeof(tree_length));
    __assert__(err);

    err = huf_alloc((void**) &tree_head, sizeof(*tree_head), tree_length);
    __assert__(err);

    err = huf_read(ifd, tree_head, tree_length * sizeof(*tree_head));
    __assert__(err);

    tree_shadow = tree_head;
    err = __huf_deserialize_tree(&root, &tree_shadow, tree_head + tree_length);
    __assert__(err);

    hctx->root = root;
    hctx->last_node= 0;
    hctx->wctx.blk_pos = 0;

    do {
        obtained = read(args.ifd, buf, __HUFFMAN_DEFAULT_BUFFER);
        if (obtained <= 0) {
            break;
        }

        err = __huf_decode_partial(hctx, &args, buf, obtained, &written);
        __assert__(err);

        if_length -= written;
        total += obtained;

    } while(total < args.len);

    if (total < args.len) {
    }

    err = __huf_decode_flush(hctx, &args, if_length);
    __assert__(err);

    __finally__;

    free(tree_head);

    __end__;
}
