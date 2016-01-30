#include <unistd.h>

#include "huffman/io.h"
#include "huffman/sys.h"
#include "huffman/malloc.h"


// Initialize a new instance of the read-writer.
huf_error_t
huf_read_writer_init(
        huf_read_writer_t **self,
        huf_reader_t reader,
        huf_writer_t writer)
{
    __try__;

    huf_error_t err;
    huf_read_writer_t *self_ptr;

    __argument__(self);

    err = huf_malloc(void_pptr_m(self),
            sizeof(huf_read_writer_t), 1);
    __assert__(err);

    self_ptr = *self;

    self_ptr->reader = reader;
    self_ptr->writer = writer;

    __finally__;
    __end__;
}


// Release memory occupied by the read-writer.
huf_error_t
huf_read_writer_free(huf_read_writer_t **self)
{
    __try__;

    __argument__(self);

    free(*self);

    *self = NULL;

    __finally__;
    __end__;
}


// Write the specified amount of byte from the buffer
// starting from the *buf* pointer.
huf_error_t
huf_write(huf_writer_t writer, const void *buf, size_t count)
{
    __try__;

    __argument__(buf);

    size_t have_written = write(writer, buf, count);
    if (have_written < 0) {
        __raise__(HUF_ERROR_READ_WRITE);
    }

    __finally__;
    __end__;
}


// Read the specified amount of bytes into the buffer
// starting from the *buf* pointer.
huf_error_t
huf_read(huf_reader_t reader, void *buf, size_t *count)
{
    __try__;

    __argument__(buf);
    __argument__(count);

    size_t have_read = read(reader, buf, *count);
    if (have_read < 0) {
        *count = 0;
        __raise__(HUF_ERROR_READ_WRITE);
    }

    *count = have_read;

    __finally__;
    __end__;
}
