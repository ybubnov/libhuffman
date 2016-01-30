#ifndef INCLUDE_huffman_io_h__
#define INCLUDE_huffman_io_h__

#include "huffman/common.h"
#include "huffman/errors.h"


// huf_writer_t is a writer abstraction.
typedef int huf_writer_t;


// huf_reader_r is a reader abstraction.
typedef int huf_reader_t;


// huf_read_writer_t groups reader and
// writer abstractions.
typedef struct __huf_read_writer {
    // A reader instance.
    huf_reader_t reader;

    // A writer instance.
    huf_writer_t writer;
} huf_read_writer_t;


// Initialize a new instance of the read-writer.
huf_error_t
huf_read_writer_init(
        huf_read_writer_t **self,
        huf_reader_t reader,
        huf_writer_t writer);


// Release memory occupied by the read-writer.
huf_error_t
huf_read_writer_free(
        huf_read_writer_t **self);


// Write the specified amount of byte from the buffer
// starting from the *buf* pointer.
huf_error_t
huf_write(huf_writer_t writer, const void *buf, size_t count);


// Read the specified amount of bytes into the buffer
// starting from the *buf* pointer.
huf_error_t
huf_read(huf_reader_t reader, void *buf, size_t *count);


#endif // INCLUDE_huffman_io_h__
