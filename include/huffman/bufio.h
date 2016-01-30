#ifndef INCLUDE_huffman_bufio_h__
#define INCLUDE_huffman_bufio_h__

#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/io.h"


// huf_bufio_read_writer_t represents read/writer buffer.
typedef struct __huf_bufio_read_writer {
    // Byte sized read-write buffer.
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


// huf_bit_read_writer_t represents read/writer bit buffer.
typedef struct __huf_bit_read_writer {
    // Bit sized read/write buffer.
    uint8_t bits;

    // Current offset for bit buffer.
    uint8_t offset;
} huf_bit_read_writer_t;


// Write the first bit of the specified word into the
// bit buffer.
inline void
huf_bit_write(huf_bit_read_writer_t *self, uint8_t bit)
{
    self->offset -= self->offset ? 1 : 0;
    self->bits |= (bit & 1) << self->offset;
}


// Reset the content of the buffer.
inline void
huf_bit_read_writer_reset(huf_bit_read_writer_t *self)
{
    self->bits = 0;
    self->offset = 8;
}


// Initialize a new instance of the read-write buffer
// with the specified size in bytes.
huf_error_t
huf_bufio_read_writer_init(
        huf_bufio_read_writer_t **self,
        huf_read_writer_t *read_writer,
        size_t size);


// Release memory occupied by the read-write buffer.
huf_error_t
huf_bufio_read_writer_free(
        huf_bufio_read_writer_t **self);


// Flush the writer buffer.
huf_error_t
huf_bufio_read_writer_flush(
        huf_bufio_read_writer_t *self);


// Write the specified amount of bytes starting from the
// provided pointer into the writer buffer. If the buffer
// will be filled during the copying of bytes, it could be
// flushed.
huf_error_t
huf_bufio_write(
        huf_bufio_read_writer_t *self,
        const void *buf, size_t size);


// Read the specified amount of bytes from the reader buffer
// starting from the provided pointer.
huf_error_t
huf_bufio_read(
        huf_bufio_read_writer_t *self,
        void *buf, size_t size);


// Read the 8-bits word from the reader buffer into the
// specified pointer.
huf_error_t
huf_bufio_read_uint8(
        huf_bufio_read_writer_t *self,
        uint8_t *byte);


// Write the specified 8-bits word into the writer buffer.
huf_error_t
huf_bufio_write_uint8(
        huf_bufio_read_writer_t *self,
        uint8_t byte);


#endif // INCLUDE_huffman_bufio_h__
