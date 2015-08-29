#ifndef INCLUDE_huffman_decode_h__
#define INCLUDE_huffman_decode_h__

#include "huffman/bufio.h"
#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/io.h"
#include "huffman/tree.h"


typedef struct __huf_decoder {
    /* huffman_tree stores leaves and the root of
     * the Huffman tree.
     */
    huf_tree_t *huffman_tree;

    /* last_node stores pointer of the last decoded byte.
     */
    huf_node_t *last_node;

    /* Buffer for write operations.
     */
    huf_bufio_read_writer_t *bufio_writer;

    /* Buffer for read opearions.
     */
    huf_bufio_read_writer_t *bufio_reader;
} huf_decoder_t;


/* Function huf_decoder_init creates a new context for huffman decompressor.
 * Created instance should be deleted with huf_decoder_free.
 */
huf_error_t
huf_decoder_init(huf_decoder_t **self, huf_read_writer_t *read_writer);


/* Function huf_decoder_free releases allocated memory.
 */
huf_error_t
huf_decoder_free(huf_decoder_t **self);


/* Function huf_decode decompress data of the specified length from the
 * ifd file desciptor and writes it into the ofd file descriptor.
 */
huf_error_t
huf_decode(huf_reader_t reader, huf_writer_t writer, uint64_t len);


#endif // INCLUDE_huffman_decode_h__
