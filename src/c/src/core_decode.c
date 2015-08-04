#include <unistd.h>
#include <stdlib.h>

#include "core.h"
#include "internal.h"
#include "runtime.h"


static huf_error_t
__huf_decode_partial(huf_ctx_t *self, const huf_args_t *args, uint8_t *buf, uint64_t len, uint64_t *written)
{
    __try__;

    uint64_t pos;
    int err;

    uint8_t *byte_rwbuf;
    uint8_t bit_rwbuf;
    uint8_t bit_offset;
    uint32_t byte_offset;

    __argument__(self);
    __argument__(args);
    __argument__(buf);
    __argument__(written);

    byte_rwbuf = self->read_writer.byte_rwbuf;
    byte_offset = self->read_writer.byte_offset;
    bit_rwbuf = self->read_writer.bit_rwbuf;
    bit_offset = self->read_writer.bit_offset;
    *written = 0;

    if (self->last_node == 0) {
        self->last_node = self->root;
    }

    for (pos = 0; pos < len; pos++) {
        bit_rwbuf = buf[pos];

        for (bit_offset = 8; bit_offset > 0; bit_offset--) {
            if ((bit_rwbuf >> (bit_offset - 1)) & 1) {
                self->last_node = self->last_node->right;
            } else {
                self->last_node = self->last_node->left;
            }

            if (self->last_node->left || self->last_node->right) {
                continue;
            }

            if (byte_offset >= __HUFFMAN_DEFAULT_BUFFER) {
                err = huf_write(args->ofd, byte_rwbuf, __HUFFMAN_DEFAULT_BUFFER);
                __assert__(err);

                *written += __HUFFMAN_DEFAULT_BUFFER;
                byte_offset = 0;
            }

            // put letter into the buffer
            byte_offset++;
            byte_rwbuf[byte_offset] = self->last_node->index;

            self->last_node = self->root;
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
__huf_decode_flush(const huf_ctx_t *self, const huf_args_t *args, int extra)
{
    __try__;

    huf_error_t err;

    __argument__(self);
    __argument__(args);

    err = huf_write(args->ofd, self->read_writer.byte_rwbuf, extra);
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
huf_decode(huf_ctx_t* self, int ifd, int ofd, uint64_t len)
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

    __argument__(self);

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

    self->root = root;
    self->last_node= 0;
    self->read_writer.byte_offset = 0;

    do {
        obtained = read(args.ifd, buf, __HUFFMAN_DEFAULT_BUFFER);
        if (obtained <= 0) {
            break;
        }

        err = __huf_decode_partial(self, &args, buf, obtained, &written);
        __assert__(err);

        if_length -= written;
        total += obtained;

    } while(total < args.len);

    if (total < args.len) {
    }

    err = __huf_decode_flush(self, &args, if_length);
    __assert__(err);

    __finally__;

    free(tree_head);

    __end__;
}
