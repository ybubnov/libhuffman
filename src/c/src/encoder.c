#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "huffman/encoder.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


static huf_error_t
__huf_create_tree_by_histogram(huf_encoder_t *self)
{
    __try__;

    huf_error_t err;
    huf_node_t *shadow_tree[512];

    size_t j;
    int64_t rate, rate1, rate2;
    int16_t index1, index2;
    int16_t node = __HUFFMAN_ASCII_SYMBOLS;

    size_t start;
    uint64_t rates;

    __argument__(self);

    // Create local copies of the histogram instance attibutes.
    start = self->histogram->start;
    rates = self->histogram->frequencies;

    while (start < 512) {
        index1 = index2 = -1;
        rate1 = rate2 = 0;

        // Skip zero-value frequencies, since they are not
        // paricipating in the building of the Huffman tree.
        while (!rates[start]) {
            start++;
        }

        // Find next minimum frequencies to construct a new tree node.
        for (j = start; j < node; j++) {
            rate = rates[j];

            // Skip zero-value frequencies.
            if (!rate) {
                continue;
            }

            if (!rate1) {
                // Initialize the first frequecy value first.
                rate1 = rate;
                index1 = j;
            } else if (rate <= rate1) {
                // Swap values of the rate1 and rate2.
                rate2 = rate1;
                rate1 = rate;
                index2 = index1;
                index1 = j;
            } else if (!rate2 || rate <= rate2) {
                // If the rate2 is lower, than found.
                rate2 = rate;
                index2 = j;
            }
        }

        // Tree is constructed, leave the while loop.
        if (index1 == -1 || index2 == -1) {
            self->huffman_tree->root = shadow_tree[node - 1];
            break;
        }

        // Allocate memory for the left child of the node.
        err = huf_malloc((void**) &shadow_tree[index1], sizeof(huf_node_t), 1);
        __assert__(err);

        // Allocate memory for the right child of the node.
        err = huf_malloc((void**) &shadow_tree[index2], sizeof(huf_node_t), 1);
        __assert__(err);

        // Allocate memory for the node itself.
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

    __end__;
}


static huf_error_t
__huf_create_char_coding(huf_encoder_t *self)
{
    __try__;

    huf_error_t err;
    huf_node_t *node = NULL;
    huf_symbol_mapping_element_t *element = NULL;

    int index;
    int position;

    uint8_t coding[__1KIB_BUFFER] = {0};

    __argument__(self);

    for (index = 0; index < self->mapping->length; index++) {
        node = self->huffman_tree->leaves[index];
        position = sizeof(coding);

        if (!node) {
            continue;
        }

        // Print node to the string.
        err = huf_node_to_string(node, coding, &position);
        __assert__(err);

        // Create mapping element and inialize it with coding string.
        err = huf_symbol_mapping_element_init(&element, coding, position);
        __assert__(err);

        // Insert coding element to the symbol-aware position.
        err = huf_symbol_mapping_insert(self->mapping, index, element);
        __assert__(err);

        printf("%d\t%s\n", index, self->mapping->symbols[index].encoding);
    }

    __finally__;
    __end__;
}


static huf_error_t
__huf_encode_partial(huf_encoder_t* self, const uint8_t *buf, uint64_t len)
{
    __try__;

    huf_error_t err;
    huf_symbol_mapping_element_t *element = NULL;

    uint64_t pos;
    size_t index;

    uint8_t bit_rwbuf = 0;
    size_t bit_offset = 0;

    __argument__(self);
    /*__argument__(self->char_coding);*/

    bit_rwbuf = self->bufio_writer->bit_rwbuf;
    bit_offset = self->bufio_writer->bit_offset;

    for (pos = 0; pos < len; pos++) {
        // Retrieve the next symbol coding element.
        err = huf_symbol_mapping_get(self->mapping, buf[pos], &element);
        __assert__(err);

        for (index = element.length; index > 0; index--) {
            // Fill the next bit of the encoded byte.
            bit_rwbuf |= ((element.coding[index - 1] & 1) << bit_offset);

            if (bit_offset) {
                bit_offset--;
                continue;
            }

            // If buffer is full, the dump it to the writer buffer.
            err = huf_bufio_write_uint8(self->bufio_writer, bit_rwbuf);
            __assert__(err);

            bit_rwbuf = 0;
            bit_offset = 7;
        }
    }

    __finally__;

    self->bufio_writer->bit_rwbuf = bit_rwbuf;
    self->bufio_writer->bit_offset = bit_offset;

    __end__;
}


static huf_error_t
__huf_encode_flush(huf_encoder_t *self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    if (self->bufio_writer->bit_offset != 7) {
        printf("FLUSH\t=>\t%x\n", self->bufio_writer->bit_rwbuf);

        err = huf_bufio_write_uint8(self->bufio_writer, self->bufio_writer->bit_rwbuf);
        __assert__(err);
    }

    __finally__;
    __end__;
}


huf_error_t
huf_encoder_init(huf_encoder_t **self, const huf_encoder_config *config)
{
    __try__;

    huf_error_t err;
    huf_read_writer_t read_writer;
    huf_encoder_t *self_ptr = NULL;

    __argument__(self);
    __argument__(config);

    err = huf_malloc((void**) &self_ptr, sizeof(huf_encoder_t), 1);
    __assert__(err);

    *self = self_ptr;

    // Initialize read-writer instance.
    read_writer.reader = config->reader;
    read_writer.writer = config->writer;
    read_writer.len = config->length;

    // Allocate memory for Huffman tree.
    err = huf_tree_init(&self_ptr->huffman_tree);
    __assert__(err);

    err = huf_symbol_mapping_init(&self_ptr->mapping, __HUFFMAN_ASCII_SYMBOLS);
    __assert__(err);

    // Allocate memory for the frequency histogram.
    err = huf_histogram_init(&self_ptr->histogram, 1, __HUFFMAN_HISTOGRAM_LENGTH);
    __assert__(err);

    // Create buffered writer instance. If writer buffer size set to zero,
    // the 64 KiB buffer will be used by default.
    err = huf_bufio_read_writer_init(&self_ptr->bufio_writer, read_writer, config->writer_buffer_size);
    __assert__(err);

    // Create buffered reader instance. If reader buffer size set to zero,
    // the 64 KiB buffer will be used by default.
    err = huf_bufio_read_writer_init(&self_ptr->bufio_reader, read_writer, config->reader_buffer_size);
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

    err = huf_bufio_read_writer_free(&self_ptr->bufio_writer);
    __assert__(err);

    err = huf_bufio_read_writer_free(&self_ptr->bufio_reader);
    __assert__(err);

    err = huf_histogram_free(&self_ptr->histogram);
    __assert__(err);

    err = huf_symbol_mapping_free(&self_ptr->mapping);
    __assert__(err);

    free(self_ptr);

    *self = NULL;

    __finally__;
    __end__;
}


huf_error_t
huf_encode(huf_reader_t reader, huf_writer_t writer, huf_encoder_config *config)
{
    __try__;

    huf_error_t err;
    huf_encoder_t *self = NULL;

    uint8_t buf[__64KIB_BUFFER] = {0};
    int16_t tree_head[__HUFFMAN_MAX_TREE_LENGTH] = {0};

    int16_t actual_tree_length = 0;
    size_t tree_length = 0;

    err = huf_encoder_init(&self, config);
    __assert__(err);

    err = __huf_create_tree_by_histogram(self);
    __assert__(err);

    err = __huf_create_char_coding(self);
    __assert__(err);

    printf("ROOT %d\n", self->huffman_tree->root->index);

    // Write serialized tree into buffer.
    err = huf_tree_serialize(self->huffman_tree, tree_head, &tree_length);
    __assert__(err);

    actual_tree_length = tree_length;

    err = huf_bufio_write(self->bufio_writer, &len, sizeof(uint64_t));
    __assert__(err);

    err = huf_bufio_write(self->bufio_writer, &actual_tree_length, sizeof(int16_t));
    __assert__(err);

    err = huf_bufio_write(self->bufio_writer, tree_head, tree_length * sizeof(int16_t));
    __assert__(err);

    size_t left_to_read = len;
    size_t need_to_read;

    while (left_to_read) {
        need_to_read = self->bufio_writer->capacity;

        if (left_to_read - need_to_read < 0) {
            need_to_read = left_to_read;
        }

        err = huf_bufio_read(self->bufio_reader, buf, need_to_read);
        __assert__(err);

        err = __huf_encode_partial(self, buf, need_to_read);
        __assert__(err);

        left_to_read -= need_to_read;
    }

    err = __huf_encode_flush(self, &read_writer);
    __assert__(err);

    // Flush buffer to the file.
    err = huf_bufio_read_writer_flush(self->bufio_writer);
    __assert__(err);

    __finally__;

    huf_encoder_free(&self);

    __end__;
}
