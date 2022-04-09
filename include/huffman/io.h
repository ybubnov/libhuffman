#ifndef INCLUDE_huffman_io_h__
#define INCLUDE_huffman_io_h__

#include "huffman/common.h"
#include "huffman/errors.h"

#define CFFI_huffman_io_h__


// huf_read_writer_t groups reader and writer abstractions.
typedef struct __huf_read_writer {
    void *stream;

    // Write the specified amount of byte from the buffer
    // starting from the buf pointer.
    huf_error_t (*write)(void *stream, const void *buf, size_t count);

    // Read the count of bytes into the buffer starting from the buf pointer.
    // The amount of read bytes are written into count argument.
    huf_error_t (*read)(void *stream, void *buf, size_t *count);
} huf_read_writer_t;


huf_error_t huf_memopen(huf_read_writer_t **self, void **buf, size_t capacity);
huf_error_t huf_memlen(huf_read_writer_t *self, size_t *len);
huf_error_t huf_memclose(huf_read_writer_t **self);

huf_error_t huf_fdopen(huf_read_writer_t **self, int fd);
huf_error_t huf_fdclose(huf_read_writer_t **self);


#undef CFFI_huffman_io_h__
#endif // INCLUDE_huffman_io_h__
