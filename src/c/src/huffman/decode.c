#include <unistd.h>
#include <stdlib.h>

#include <huffman/huffman.h>
#include <huffman/runtime/malloc.h>
#include <huffman/runtime/sys.h>


static huf_error_t
__huf_decode_partial(huf_archiver_t *self, const huf_read_writer_t *read_writer, uint8_t *buf, uint64_t len, uint64_t *written)
{
    __try__;

    uint64_t pos;
    int err;

    uint8_t *byte_rwbuf;
    uint8_t bit_rwbuf;
    uint8_t bit_offset;
    uint32_t byte_offset;

    __argument__(self);
    __argument__(read_writer);
    __argument__(buf);
    __argument__(written);

    byte_rwbuf = self->bufio_read_writer.byte_rwbuf;
    byte_offset = self->bufio_read_writer.byte_offset;
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
                err = huf_write(read_writer->writer, byte_rwbuf, __HUFFMAN_DEFAULT_BUFFER);
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

    self->bufio_read_writer.byte_rwbuf = byte_rwbuf;
    self->bufio_read_writer.byte_offset = byte_offset;

    __end__;
}


static huf_error_t
__huf_decode_flush(const huf_archiver_t *self, const huf_read_writer_t *read_writer, int extra)
{
    __try__;

    huf_error_t err;

    __argument__(self);
    __argument__(read_writer);

    err = huf_write(read_writer->writer, self->bufio_read_writer.byte_rwbuf, extra);
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
        err = huf_malloc((void**) node, sizeof(huf_node_t), 1);
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
huf_decode(huf_archiver_t* self, huf_reader_t reader, huf_writer_t writer, uint64_t len)
{
    __try__;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];
    uint64_t reader_length, written;

    int16_t tree_length = 0;
    int16_t *tree_head, *tree_shadow;

    size_t left_to_read = len;
    size_t need_to_read;

    huf_node_t* root;
    huf_error_t err;
    huf_read_writer_t read_writer = {reader, writer, len};

    __argument__(self);

    need_to_read = sizeof(reader_length);
    left_to_read -= need_to_read;
    err = huf_read(reader, &reader_length, &need_to_read);
    __assert__(err);

    need_to_read = sizeof(tree_length);
    left_to_read -= need_to_read;
    err = huf_read(reader, &tree_length, &need_to_read);
    __assert__(err);

    err = huf_malloc((void**) &tree_head, sizeof(*tree_head), tree_length);
    __assert__(err);

    need_to_read = tree_length * sizeof(*tree_head);
    left_to_read -= need_to_read;
    err = huf_read(reader, tree_head, &need_to_read);
    __assert__(err);

    tree_shadow = tree_head;
    err = __huf_deserialize_tree(&root, &tree_shadow, tree_head + tree_length);
    __assert__(err);

    self->root = root;
    self->last_node= 0;
    self->bufio_read_writer.byte_offset = 0;

    do {
        need_to_read = __HUFFMAN_DEFAULT_BUFFER;

        if (left_to_read - need_to_read < 0) {
            need_to_read = left_to_read;
        }

        err = huf_read(read_writer.reader, buf, &need_to_read);
        __assert__(err);

        err = __huf_decode_partial(self, &read_writer, buf, need_to_read, &written);
        __assert__(err);

        left_to_read -= need_to_read;
        reader_length -= written;
    } while (left_to_read);

    err = __huf_decode_flush(self, &read_writer, reader_length);
    __assert__(err);

    __finally__;

    free(tree_head);

    __end__;
}
