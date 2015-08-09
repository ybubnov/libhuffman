#ifndef INCLUDE_huffman_bufio_h__
#define INCLUDE_huffman_bufio_h__

#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/io.h"


/* bufio_read_writer represents bit-grained read/writer buffer.
 */
typedef struct __huf_bufio_read_writer {
    /* Byte sized read/write buffer.
     */
    uint8_t *byte_rwbuf;

    /* Current offset for byte buffer.
     */
    uint32_t byte_offset;

    /* Bit sized read/write buffer.
     */
    uint8_t bit_rwbuf;

    /* Current offset for bit buffer.
     */
    uint8_t bit_offset;

    /* Size in bytes of the buffer.
     */
    size_t size;

    /* Count of bytes have been written to writer.
     */
    uint64_t have_been_written;

    /* Read-Writer instance.
     */
    huf_read_writer_t *read_writer;
} huf_bufio_read_writer_t;


huf_error_t
huf_bufio_read_writer_init(huf_bufio_read_writer_t **self, huf_read_writer_t *read_writer, size_t size);


huf_error_t
huf_bufio_read_writer_free(huf_bufio_read_writer_t **self);


huf_error_t
huf_bufio_read_writer_flush(huf_bufio_read_writer_t *self, size_t size);


huf_error_t
huf_bufio_write_uint8(huf_bufio_read_writer_t *self, uint8_t byte);


#endif // INCLUDE_huffman_bufio_h__
