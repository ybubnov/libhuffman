#ifndef INCLUDE_huffman_config_h__
#define INCLUDE_huffman_config_h__

#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/io.h"


typedef struct __huf_encoder_config {
    // Count of the reader bytes to encode. This is the only
    // mandatory parameter, if set to zero then no data will
    // be compressed.
    uint64_t length;

    // Size of the encoding chunk. If set to zero then
    // length of the data to encode will be treated as
    // size of the chunk.
    uint64_t chunk_size;

    // Size of the reader buffer in bytes. If set to zero
    // then will be defaulted to 64 KiB.
    size_t reader_buffer_size;

    // Size of the writer buffer in bytes. If set to zero
    // then will be defaulted to 64 KiB.
    size_t writer_buffer_size;

    // Instance of the reader which will be used as
    // a provider of the input data.
    huf_reader_t reader;

    // Instance of the writer which will be used as
    // a consumer of the huffman-encoded data.
    huf_writer_t writer;
} huf_encoder_config_t;


huf_error_t
huf_encoder_config_init(huf_encoder_config_t **self);


huf_error_t
huf_encoder_config_free(huf_encoder_config_t **self);


typedef struct __huf_decoder_config {
    // Count of the reader bytes to encode. This is the only
    // mandatory parameter, if set to zero then no data will
    // be compressed.
    uint64_t length;

    // Size of the reader buffer in bytes. If set to zero
    // then will be defaulted to 64 KiB.
    size_t reader_buffer_size;

    // Size of the writer buffer in bytes. If set to zero
    // then will be defaulted to 64 KiB.
    size_t writer_buffer_size;

    // Instance of the reader which will be used as
    // a provider of the input data.
    huf_reader_t reader;

    // Instance of the writer which will be used as
    // a consumer of the huffman-encoded data.
    huf_writer_t writer;
} huf_decoder_config_t;


huf_error_t
huf_decoder_config_init(huf_decoder_config_t **self);


huf_error_t
huf_decoder_config_free(huf_decoder_config_t **self);


#endif // INCLUDE_huffman_config_h__
