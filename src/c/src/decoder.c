#include <unistd.h>
#include <stdlib.h>

#include "huffman/decoder.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


static huf_error_t
__huf_decode_partial(huf_decoder_t *self, const uint8_t *buf, size_t len)
{
    __try__;

    huf_error_t err;

    uint8_t byte;
    uint8_t bit_offset;

    size_t position;

    __argument__(self);
    __argument__(buf);

    if (!self->last_node) {
        self->last_node = self->huffman_tree->root;
    }

    for (position = 0; position < len; position++) {
        // Read the next chunk of bit stream.
        byte = buf[position];

        for (bit_offset = 8; bit_offset > 0; bit_offset--) {
            // If next bit equals to 1, then move to the right branch.
            // Otherwise move to the left branch.
            if ((byte >> (bit_offset - 1)) & 1) {
                self->last_node = self->last_node->right;
            } else {
                self->last_node = self->last_node->left;
            }

            // Continue until the leaf (encoded byte) will be found.
            if (self->last_node->left || self->last_node->right) {
                continue;
            }

            err = huf_bufio_write_uint8(self->bufio_writer, self->last_node->index);
            __assert__(err);

            // Reset last node value to tree root.
            self->last_node = self->huffman_tree->root;
        }
    }

    __finally__;
    __end__;
}


huf_error_t
huf_decoder_init(huf_decoder_t **self, huf_read_writer_t *read_writer)
{
    __try__;

    huf_error_t err;
    huf_decoder_t *self_ptr;

    __argument__(self);
    __argument__(read_writer);

    err = huf_malloc((void**) &self_ptr, sizeof(huf_decoder_t), 1);
    __assert__(err);

    *self = self_ptr;

    // Allocate memory for Huffman tree.
    err = huf_tree_init(&self_ptr->huffman_tree);
    __assert__(err);

    // Create buffered writer instance with 64 KiB buffer.
    err = huf_bufio_read_writer_init(&self_ptr->bufio_writer, read_writer, __HUFFMAN_DEFAULT_BUFFER);
    __assert__(err);

    // Create buffered readear instance with 64 KiB buffer.
    err = huf_bufio_read_writer_init(&self_ptr->bufio_reader, read_writer, __HUFFMAN_DEFAULT_BUFFER);
    __assert__(err);

    __finally__;
    __end__;
}


huf_error_t
huf_decoder_free(huf_decoder_t **self)
{
    __try__;

    huf_error_t err;
    huf_decoder_t *self_ptr;

    __argument__(self);

    self_ptr = *self;

    err = huf_tree_free(&self_ptr->huffman_tree);
    __assert__(err);

    err = huf_bufio_read_writer_free(&self_ptr->bufio_writer);
    __assert__(err);

    err = huf_bufio_read_writer_free(&self_ptr->bufio_reader);
    __assert__(err);

    free(self_ptr);

    *self = NULL;

    __finally__;
    __end__;
}


huf_error_t
huf_decode(huf_reader_t reader, huf_writer_t writer, uint64_t len)
{
    __try__;

    huf_read_writer_t read_writer = {reader, writer, len};
    huf_decoder_t *self = NULL;

    huf_error_t err;

    uint8_t buf[__HUFFMAN_DEFAULT_BUFFER];
    uint64_t reader_length = 0;

    int16_t *tree_head = NULL;
    int16_t tree_length = 0;

    size_t left_to_read = len;
    size_t need_to_read;

    // Create decoder instance.
    err = huf_decoder_init(&self, &read_writer);
    __assert__(err);

    // Read the length of the original file.
    err = huf_bufio_read(self->bufio_reader, &reader_length, sizeof(reader_length));
    __assert__(err);

    // Read the length of the huffman tree.
    err = huf_bufio_read(self->bufio_reader, &tree_length, sizeof(tree_length));
    __assert__(err);

    // Allocate memory for serialized huffman tree.
    err = huf_malloc((void**) &tree_head, sizeof(*tree_head), tree_length);
    __assert__(err);

    // Read serialized huffman tree.
    err = huf_bufio_read(self->bufio_reader, tree_head, tree_length * sizeof(*tree_head));
    __assert__(err);

    // Create linked tree strcuture.
    err = huf_tree_deserialize(self->huffman_tree, tree_head, tree_length);
    __assert__(err);

    left_to_read -= self->bufio_reader->have_been_processed;

    while (left_to_read > 0) {
        need_to_read = self->bufio_writer->capacity;

        if (left_to_read < need_to_read) {
            need_to_read = left_to_read;
        }

        err = huf_bufio_read(self->bufio_reader, buf, need_to_read);
        __assert__(err);

        __debug__("HERE\n");

        err = __huf_decode_partial(self, buf, need_to_read);
        __assert__(err);

        left_to_read -= need_to_read;
    }

    __debug__("EXIT\n");

    /*err = huf_bufio_writer_flush(self->bufio_writer, reader_length);*/
    err = huf_bufio_read_writer_flush(self->bufio_writer);
    __assert__(err);

    __finally__;

    huf_decoder_free(&self);
    free(tree_head);

    __end__;
}
