#ifndef HUFFMAN_H
#define HUFFMAN_H

#define _LARGEFILE64_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdint.h>

/* Default buffer size for write operations.
 */
#define __HUFFMAN_DEFAULT_BUFFER 65536
#define __HUFFMAN_ASCII_SYMBOLS 256

/* All leafs of the huffman tree will be marked with that value.
 */
#define __HUFFMAN_LEAF 1024


typedef struct __huf_node {
    int16_t index;
    struct __huf_node_t* parent;
    struct __huf_node_t* left;
    struct __huf_node_t* right;
} huf_node_t;

typedef struct __huf_table_ctx {
    int length;
    char* encoding;
} huf_table_ctx_t;

typedef struct __huf_write_ctx {
    uint8_t blk_buf[__HUFFMAN_DEFAULT_BUFFER];
    uint8_t bit_buf;
    uint8_t bit_pos;
    uint32_t blk_pos;
} huf_write_ctx_t;

typedef struct __huf_context {
    huf_node_t **leaves;
    huf_node_t *root;
    huf_node_t *last_node;
    huf_table_cxt_t *table;
    huf_write_ctx_t wctx;
} huf_ctx_t;


/* Function huf_init creates a new context for huffman compressor.
 * Created instance should be deleted with huf_free.
 */
huf_error_t huf_init(huf_ctx_t* hctx);

/* Function huf_encode compress data of specifiled length from the
 * ifd file descriptot and writes it into the ofd file descriptor.
 */
huf_error_t huf_encode(huf_ctx_t *hctx, int ifd, int ofd, uint64_t len);

/* Function huf_decode decompress data of the specified length from the
 * ifd file desciptor and writes it into the ofd file descriptor.
 */
huf_error_t huf_decode(huf_ctx_t *hctx, int *ifd, int *dfd, uint64_t len);

/* Function huf_free releases allocated memory
 */
void huf_free(huf_ctx_t *hctx);


#endif //HUFFMAN_H
