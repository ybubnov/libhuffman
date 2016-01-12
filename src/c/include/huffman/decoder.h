#ifndef INCLUDE_huffman_decode_h__
#define INCLUDE_huffman_decode_h__

#include "huffman/bufio.h"
#include "huffman/config.h"
#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/io.h"
#include "huffman/tree.h"


typedef struct __huf_decoder {
    // Read-only field with decoder configuration.
    huf_config_t *config;

    // huffman_tree stores leaves and the root of
    // the Huffman tree.
    huf_tree_t *huffman_tree;

    // last_node stores pointer of the last decoded byte.
    huf_node_t *last_node;

    // Read-writer instance.
    huf_read_writer_t *read_writer;

    // Buffer for write operations.
    huf_bufio_read_writer_t *bufio_writer;

    // Buffer for read opearions.
    huf_bufio_read_writer_t *bufio_reader;
} huf_decoder_t;


// Function huf_decoder_init creates a new context for huffman decompressor.
// Created instance should be deleted with huf_decoder_free.
huf_error_t
huf_decoder_init(huf_decoder_t **self, const huf_config_t *config);


// Function huf_decoder_free releases allocated memory.
huf_error_t
huf_decoder_free(huf_decoder_t **self);


// Function huf_decode decompress data of the specified length from the
// ifd file desciptor and writes it into the ofd file descriptor.
huf_error_t
huf_decode(const huf_config_t *config);


#endif // INCLUDE_huffman_decode_h__
