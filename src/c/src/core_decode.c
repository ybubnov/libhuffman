#include "rt.h"
#include "core.h"
#include "internal.h"


static huf_error_t
__huf_decode_partial(const huf_ctx_t *hctx, const huf_args_t *args, uint8_t *buf, uint64_t len, uint64_t *written)
{
    __TRY__;

    uint64_t pos;
    int err;

    uint8_t *blk_buf;
    uint8_t bit_buf;
    uint8_t bit_pos;
    uint32_t blk_pos;

    huf_write_ctx_t wctx;

    __ASSERT__(!hctx, HUF_ERROR_INVALID_ARGUMENT);
    __ASSERT__(!args, HUF_ERROR_INVALID_ARGUMENT);

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
                err = write(args->ofd, blk_buf, __HUFFMAN_DEFAULT_BUFFER);
                __ASSERT_TRUE__(err == -1, HUF_ERROR_READ_WRITE);

                *written += __HUFFMAN_DEFAULT_BUFFER;
                blk_pos = 0;
            }

            // put letter into the buffer
            blk_pos++;
            blk_buf[blk_pos] = hctx->last_node->index;

            hctx->last_node = hctx->root;
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
__huf_decode_flush(const huf_ctx_t *hctx, const huf_args_t *args, int extra)
{
    __TRY__;

    int err;

    __ASSERT__(!hctx, HUF_INVALID_ARUGMENT);
    __ASSERT__(!args, HUF_INVALID_ARUGMENT);

    err = write(args->ofd, hctx->blk_buf, extra);
    __ASSERT__(err == -1, HUF_ERROR_READ_WRITE);

    __FINALLY__;
    __END__;
}


static huf_error_t
__huf_deserialize_tree(huf_node_t **node, int16_t **src, int16_t *src_end)
{
    __TRY__;

    __ASSERT__(!node, HUF_ERROR_INVALID_ARGUMENT);
    __ASSERT__(!src, HUF_ERROR_INVALID_ARGUMENT);
    __ASSERT__(!src_end, HUF_ERROR_INVALID_ARGUMENT);

    int16_t n_index = **src;
    huf_node_t **node_left;
    huf_node_t **node_right;

    huf_error_t err;

    if ((*src) + 1 > src_end) {
        __RAISE__(HUF_INVALID_ARGUMENT);
    }

    (*src)++;

    if (n_index != __HUFFMAN_LEAF_FLAG) {
        (*node) = calloc(1, sizeof(huf_node_t));
        __ASSERT__(!(*node), HUF_ERROR_MEMORY_ALLOCATION);

        (*node)->index = n_index;
        node_left = &((*node)->left);
        node_right = &((*node)->right);

        err = __huf_deserialize_tree(node_left, src, src_end);
        __ASSERT__(err, err);

        err = __huf_deserialize_tree(node_right, src, src_end);
        __ASSERT__(err, err);
    }

    __FINALLY__;
    __END__;
}


huf_error_t
huf_decode(huf_ctx_t* hctx, int ifd, int ofd, uint64_t len)
{
    __TRY__;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];
    uint64_t obtained, total = 0;
    uint64_t if_length, written;

    int16_t tree_length = 0;
    int16_t *tree_head, *tree_shadow;

    huf_node_t* root;
    huf_error_t err;
    huf_args_t args = {ifd, ofd, len};

    __ASSERT__(!hctx, HUF_ERROR_INVALID_ARGUMENT);

    obtained = read(ifd, &if_length, sizeof(length));
    __ASSERT__(obtained == -1, HUF_ERROR_READ_WRITE);

    obtained = read(ifd, &tree_length, sizeof(tree_length));
    __ASSERT__(obtained == -1, HUF_ERROR_READ_WRITE);

    tree_head = calloc(tree_length, sizeof(*tree_head));
    __ASSERT__(!tree_head, HUF_ERROR_MEMORY_ALLOCATION);

    obtained = read(ifd, tree_head, tree_length * sizeof(*tree_head));
    __ASSERT__(obtained == -1, HUF_ERROR_READ_WRITE);

    tree_shadow = tree_head;
    err = __huf_deserialize_tree(&root, &tree_shadow, tree_head + tree_length);
    __ASSERT__(err, err);

    hctx->root = root;
    hctx->last_node= 0;
    hctx->wctx->blk_pos = 0;

    do {
        obtained = read(args.ifd, buf, __HUFFMAN_DEFAULT_BUFFER);
        if (obtained <= 0) {
            break;
        }

        err = __huf_decode_partial(hctx, &args, buf, obtained, &written);
        __ASSERT__(err, err);

        if_length -= written;
        total += obtained;
    } while(total < args.len);

    err = __huf_decode_flush(hctx, &args, if_length);
    __ASSERT__(err, err);

    __FINALLY__;

    free(tree_head);

    __END__;
}
