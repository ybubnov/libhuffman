#ifndef INCLUDE_huffman_encode_h__
#define INCLUDE_huffman_encode_h__

#include "huffman/bufio.h"
#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/io.h"
#include "huffman/tree.h"


typedef struct __huf_encoder {
    /* huffman_tree stores leaves and the root of
     * the Huffman tree.
     */
    huf_tree_t *huffman_tree;

    /* char_coding represents map of binary encoding for
     * particular byte.
     */
    huf_char_coding_t *char_coding;

    /* read_writer groups read and write operations
     */
    huf_bufio_read_writer_t *bufio_read_writer;
} huf_encoder_t;


/* Function huf_encoder_init creates a new context for huffman compressor.
 * Created instance should be deleted with huf_encoder_free.
 */
huf_error_t
huf_encoder_init(huf_encoder_t **self, huf_read_writer_t *read_writer);


/* Function huf_encoder_free releases allocated memory.
 */
huf_error_t
huf_encoder_free(huf_encoder_t **self);


/* Function huf_encode compresses data of specifiled length from the
 * reader and writes it into the writer.
 */
huf_error_t
huf_encode(huf_reader_t reader, huf_writer_t writer, uint64_t len);


#endif // INCLUDE_huffman_encode_h__
