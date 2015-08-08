#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>

#include <huffman/errors.h>
#include <huffman/bufio/io.h>
#include <huffman/bufio/bufio.h>


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


typedef struct __huf_archiver {
    huf_node_t **leaves;
    huf_node_t *root;
    huf_node_t *last_node;

    /* char_coding represents map of binary encoding for
     * particular ascii symbol.
     */
    huf_char_coding_t *char_coding;

    /* read_writer groups read and write operations
     */
    huf_bufio_read_writer_t bufio_read_writer;
} huf_archiver_t;


/* Function huf_init creates a new context for huffman compressor.
 * Created instance should be deleted with huf_free.
 */
huf_error_t
huf_init(huf_archiver_t **self);


/* Function huf_free releases allocated memory
 */
huf_error_t
huf_free(huf_archiver_t **self);


/* Function huf_encode compress data of specifiled length from the
 * ifd file descriptot and writes it into the ofd file descriptor.
 */
huf_error_t
huf_encode(huf_archiver_t *self, huf_reader_t reader, huf_writer_t writer, uint64_t len);

/* Function huf_decode decompress data of the specified length from the
 * ifd file desciptor and writes it into the ofd file descriptor.
 */
huf_error_t
huf_decode(huf_archiver_t *self, huf_reader_t reader, huf_writer_t writer, uint64_t len);


#endif // HUFFMAN_H
