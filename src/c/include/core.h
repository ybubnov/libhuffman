#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>

#include "error.h"


/* Default buffer size for write operations.
 */
#define __HUFFMAN_DEFAULT_BUFFER 65536
#define __HUFFMAN_ASCII_SYMBOLS 256

/* All leafs of the huffman tree will be marked with that value.
 */
#define __HUFFMAN_LEAF 1024


typedef struct __huf_node {
    int16_t index;
    struct __huf_node* parent;
    struct __huf_node* left;
    struct __huf_node* right;
} huf_node_t;

typedef struct __huf_char_coding {
    int length;
    uint8_t* encoding;
} huf_char_coding_t;

typedef struct __huf_read_writer {
    uint8_t *byte_rwbuf;
    uint32_t byte_offset;
    uint8_t bit_rwbuf;
    uint8_t bit_offset;
} huf_read_writer_t;

typedef struct __huf_context {
    huf_node_t **leaves;
    huf_node_t *root;
    huf_node_t *last_node;
    huf_char_coding_t *table;

    /* read_writer groups read and write operations
     */
    huf_read_writer_t read_writer;
} huf_ctx_t;


/* Function huf_init creates a new context for huffman compressor.
 * Created instance should be deleted with huf_free.
 */
huf_error_t huf_init(huf_ctx_t* self);

/* Function huf_encode compress data of specifiled length from the
 * ifd file descriptot and writes it into the ofd file descriptor.
 */
huf_error_t huf_encode(huf_ctx_t *self, int ifd, int ofd, uint64_t len);

/* Function huf_decode decompress data of the specified length from the
 * ifd file desciptor and writes it into the ofd file descriptor.
 */
huf_error_t huf_decode(huf_ctx_t *self, int ifd, int ofd, uint64_t len);

/* Function huf_free releases allocated memory
 */
void huf_free(huf_ctx_t *self);


#endif //HUFFMAN_H
