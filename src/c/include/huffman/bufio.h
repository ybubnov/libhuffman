#ifndef INCLUDE_huffman_bufio_h__
#define INCLUDE_huffman_bufio_h__

#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/io.h"

// Default buffer size for write operations.
#define __HUFFMAN_DEFAULT_BUFFER 65536

// huf_bufio_read_writer_t represents read/writer buffer.
typedef struct __huf_bufio_read_writer {
    // Byte sized read/write buffer.
    uint8_t *bytes;

    // Current read position.
    size_t offset;

    // Available size in bytes of the buffer.
    size_t capacity;

    // Current length of the buffer. Will be increased on write
    // operations until this value will reach buffer capacity.
    size_t length;

    // Count of bytes have been processed by read-writer.
    uint64_t have_been_processed;

    // Read-Writer instance.
    huf_read_writer_t *read_writer;
} huf_bufio_read_writer_t;

//    /* Bit sized read/write buffer.
//     */
//    uint8_t bit_rwbuf;
//
//    /* Current offset for bit buffer.
//     */
//    uint8_t bit_offset;


huf_error_t
huf_bufio_read_writer_init(huf_bufio_read_writer_t **self, huf_read_writer_t *read_writer, size_t size);


huf_error_t
huf_bufio_read_writer_free(huf_bufio_read_writer_t **self);


huf_error_t
huf_bufio_read_writer_flush(huf_bufio_read_writer_t *self);


huf_error_t
huf_bufio_write(huf_bufio_read_writer_t *self, const void *buf, size_t size);


huf_error_t
huf_bufio_read(huf_bufio_read_writer_t *self, void *buf, size_t size);


huf_error_t
huf_bufio_write_uint8(huf_bufio_read_writer_t *self, uint8_t byte);


#endif // INCLUDE_huffman_bufio_h__
