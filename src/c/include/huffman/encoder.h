#ifndef INCLUDE_huffman_encode_h__
#define INCLUDE_huffman_encode_h__

#include "huffman/bufio.h"
#include "huffman/config.h"
#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/histogram.h"
#include "huffman/io.h"
#include "huffman/symbol.h"
#include "huffman/tree.h"


typedef struct __huf_encoder {
    // Read-only field with encoder configuration.
    huf_encoder_config_t *config;

    // Bit buffer.
    huf_bit_read_writer_t bit_writer;

    // huffman_tree stores leaves and the root of
    // the Huffman tree.
    huf_tree_t *huffman_tree;

    // char_coding represents map of binary encoding for
    // particular byte.
    huf_symbol_mapping_t *mapping;

    // Frequencies of the symbols occurence.
    huf_histogram_t *histogram;

    // Read-writer instance.
    huf_read_writer_t *read_writer;

    // bufio_writer represents buffered reader.
    huf_bufio_read_writer_t *bufio_writer;

    // bufio_reader represetnt buffered writer.
    huf_bufio_read_writer_t *bufio_reader;
} huf_encoder_t;


// Function huf_encoder_init creates a new context for huffman compressor.
// Created instance should be deleted with huf_encoder_free.
huf_error_t
huf_encoder_init(huf_encoder_t **self, const huf_encoder_config_t *config);


// Function huf_encoder_free releases allocated memory.
huf_error_t
huf_encoder_free(huf_encoder_t **self);


// Function huf_encode compresses data of specifiled length from the
// reader and writes it into the writer.
huf_error_t
huf_encode(const huf_encoder_config_t *config);


#endif // INCLUDE_huffman_encode_h__
