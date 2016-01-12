#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "huffman/encoder.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


extern void
huf_bit_write(huf_bit_read_writer_t *, uint8_t);


extern void
huf_bit_read_writer_reset(huf_bit_read_writer_t *);


static huf_error_t
__huf_create_tree_by_histogram(huf_encoder_t *self)
{
    __try__;

    huf_error_t err;
    huf_node_t *shadow_tree[512] = {0};

    size_t j;
    int64_t rate, rate1, rate2;
    int16_t index1, index2;
    int16_t node = __HUFFMAN_ASCII_SYMBOLS;

    size_t start;
    uint64_t *rates = NULL;

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

        if (!shadow_tree[index1]) {
            // Allocate memory for the left child of the node.
            err = huf_malloc((void**) &shadow_tree[index1], sizeof(huf_node_t), 1);
            __assert__(err);
        }

        if (!shadow_tree[index2]) {
            // Allocate memory for the right child of the node.
            err = huf_malloc((void**) &shadow_tree[index2], sizeof(huf_node_t), 1);
            __assert__(err);
        }

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

    size_t index;
    size_t position;

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

        printf("%d\t len - %d == %d | ", (int)index, (int)strlen((char*)coding), (int) position);

        // Insert coding element to the symbol-aware position.
        err = huf_symbol_mapping_insert(self->mapping, index, element);
        __assert__(err);

        printf("%d\t%s (%p)\n", (int)index, self->mapping->symbols[index]->coding, (void*)self->mapping->symbols[index]);
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

    __argument__(self);
    __argument__(buf);

    /*printf("POS = %d\n", (int) len);*/

    for (pos = 0; pos < len; pos++) {
        // Retrieve the next symbol coding element.
        err = huf_symbol_mapping_get(self->mapping, buf[pos], &element);
        __assert__(err);

        for (index = element->length; index > 0; index--) {
            // Fill the next bit of the encoded byte.
            huf_bit_write(&self->bit_writer, element->coding[index - 1]);

            if (self->bit_writer.offset) {
                continue;
            }

            // If buffer is full, then dump it to the writer buffer.
            err = huf_bufio_write_uint8(self->bufio_writer,
                    self->bit_writer.bits);
            __assert__(err);

            huf_bit_read_writer_reset(&self->bit_writer);
        }
    }

    __finally__;
    __end__;
}


static huf_error_t
__huf_encode_flush(huf_encoder_t *self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    if (self->bit_writer.offset != 7) {
        printf("FLUSH\t=>\t%x\n", self->bit_writer.bits);

        err = huf_bufio_write_uint8(self->bufio_writer, self->bit_writer.bits);
        __assert__(err);
    }

    __finally__;
    __end__;
}


huf_error_t
huf_encoder_init(huf_encoder_t **self, const huf_config_t *config)
{
    __try__;

    huf_error_t err;
    huf_encoder_t *self_ptr = NULL;
    huf_config_t *encoder_config = NULL;

    __argument__(self);
    __argument__(config);

    err = huf_malloc((void**) &self_ptr, sizeof(huf_encoder_t), 1);
    __assert__(err);

    *self = self_ptr;

    // Save the encoder configuration.
    err = huf_config_init(&encoder_config);
    __assert__(err);

    memcpy(encoder_config, config, sizeof(*config));

    // If size of encoding chunk set to zero then length of the
    // data to encode will be treated as size of the chunk.
    if (!encoder_config->chunk_size) {
        encoder_config->chunk_size = encoder_config->length;
    }

    self_ptr->config = encoder_config;

    // Initialize read-writer instance.
    err = huf_read_writer_init(&self_ptr->read_writer,
            self_ptr->config->reader, config->writer);
    __assert__(err);

    // Allocate memory for Huffman tree.
    err = huf_tree_init(&self_ptr->huffman_tree);
    __assert__(err);

    err = huf_symbol_mapping_init(&self_ptr->mapping,
            __HUFFMAN_ASCII_SYMBOLS);
    __assert__(err);

    // Allocate memory for the frequency histogram.
    err = huf_histogram_init(&self_ptr->histogram, 1,
            __HUFFMAN_HISTOGRAM_LENGTH);
    __assert__(err);

    // Create buffered writer instance. If writer buffer size set to zero,
    // the 64 KiB buffer will be used by default.
    err = huf_bufio_read_writer_init(&self_ptr->bufio_writer,
            self_ptr->read_writer, self_ptr->config->writer_buffer_size);
    __assert__(err);

    // Create buffered reader instance. If reader buffer size set to zero,
    // the 64 KiB buffer will be used by default.
    err = huf_bufio_read_writer_init(&self_ptr->bufio_reader,
            self_ptr->read_writer, self_ptr->config->reader_buffer_size);
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

    err = huf_read_writer_free(&self_ptr->read_writer);
    __assert__(err);

    err = huf_histogram_free(&self_ptr->histogram);
    __assert__(err);

    err = huf_symbol_mapping_free(&self_ptr->mapping);
    __assert__(err);

    err = huf_config_free(&self_ptr->config);
    __assert__(err);

    free(self_ptr);

    *self = NULL;

    __finally__;
    __end__;
}



static void
__show_tree(huf_node_t *node, int spaces);

static void
__show_tree(huf_node_t *node, int spaces)
{
    char *buf = calloc(spaces+1, sizeof(char));
    memset(buf, 45, sizeof(char) * spaces);

    if (node) {
        printf("%s%d\n", buf, node->index);
    } else {
        printf("%s%d\n", buf, 0);
    }

    free(buf);
    if (!node) {
        return;
    }

    spaces += 2;

    printf("(l %d) ", spaces);
    __show_tree(node->left, spaces);
    printf("(r %d) ", spaces);
    __show_tree(node->right, spaces);
}

huf_error_t
huf_encode(const huf_config_t *config)
{
    __try__;

    huf_error_t err;
    huf_encoder_t *self = NULL;

    uint8_t *buf = NULL;
    int16_t tree_head[__HUFFMAN_MAX_TREE_LENGTH] = {0};

    int16_t actual_tree_length = 0;
    size_t tree_length = 0;

    err = huf_encoder_init(&self, config);
    __assert__(err);

    err = huf_malloc((void**) &buf, sizeof(uint8_t),
            self->config->chunk_size);
    __assert__(err);

    size_t left_to_read = self->config->length;
    size_t need_to_read;

    while (left_to_read > 0) {
        need_to_read = self->config->chunk_size;

        if (left_to_read < need_to_read) {
            need_to_read = left_to_read;
        }

        // Read the next chunk of data, that we are going to encode.
        err = huf_bufio_read(self->bufio_reader, buf, need_to_read);
        __assert__(err);

        err = huf_histogram_populate(self->histogram, buf, need_to_read);
        __assert__(err);

        err = __huf_create_tree_by_histogram(self);
        __assert__(err);

        err = __huf_create_char_coding(self);
        __assert__(err);

        printf("ROOT %d\n", self->huffman_tree->root->index);
        __show_tree(self->huffman_tree->root, 1);

        // Write serialized tree into buffer.
        err = huf_tree_serialize(self->huffman_tree,
                tree_head, &tree_length);
        __assert__(err);

        actual_tree_length = tree_length;

        // Write the size of the next chunk.
        err = huf_bufio_write(self->bufio_writer,
                &need_to_read, sizeof(uint64_t));
        __assert__(err);

        // Write the length of the serialized Huffman tree.
        err = huf_bufio_write(self->bufio_writer,
                &actual_tree_length, sizeof(int16_t));
        __assert__(err);

        // Write the serialized tree itself.
        err = huf_bufio_write(self->bufio_writer, tree_head,
                tree_length * sizeof(int16_t));
        __assert__(err);

        printf("AFTER TREE WRITE = %lld\n", (long long) self->bufio_writer->length);
        printf("NEED TO READ = %lld\n", (long long) need_to_read);

        // Write data
        err = __huf_encode_partial(self, buf, need_to_read);
        __assert__(err);

        left_to_read -= need_to_read;

        // If there is no more data to read, then skip reset of the histogram.
        if (!left_to_read) {
            continue;
        }

        err = huf_tree_reset(self->huffman_tree);
        __assert__(err);

        err = huf_histogram_reset(self->histogram);
        __assert__(err);

        err = huf_symbol_mapping_reset(self->mapping);
        __assert__(err);
    }

    printf("HAVE WRITTEN = %lld\n", (long long)self->bufio_writer->have_been_processed);
    printf("LENGTH = %lld\n", (long long)self->bufio_writer->length);

    err = __huf_encode_flush(self);
    __assert__(err);

    // Flush buffer to the file.
    err = huf_bufio_read_writer_flush(self->bufio_writer);
    __assert__(err);

    __finally__;

    huf_encoder_free(&self);
    free(buf);

    __end__;
}
