#include "huffman/bufio.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


huf_error_t
huf_bufio_read_writer_init(huf_bufio_read_writer_t **self, huf_read_writer_t *read_writer, size_t size)
{
    __try__;

    huf_bufio_read_writer_t *self_pointer;
    huf_error_t err;

    __argument__(self);

    err = huf_malloc((void**) self, sizeof(huf_bufio_read_writer_t), 1);
    __assert__(err);

    self_pointer = *self;

    err = huf_malloc((void**) &self_pointer->byte_rwbuf, sizeof(uint8_t), size);
    __assert__(err);

    self_pointer->size = size;
    self_pointer->read_writer = read_writer;

    __finally__;
    __end__;
}


huf_error_t
huf_bufio_read_writer_free(huf_bufio_read_writer_t **self)
{
    __try__;

    huf_bufio_read_writer_t *self_pointer;

    __argument__(self);

    self_pointer = *self;

    free(self_pointer->byte_rwbuf);
    free(self_pointer);

    *self = 0;

    __finally__;
    __end__;
}


huf_error_t
huf_bufio_read_writer_flush(huf_bufio_read_writer_t *self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    err = huf_write(self->read_writer->writer, self->byte_rwbuf, self->byte_offset);
    __assert__(err);

    self->have_been_written += self->byte_offset;

    __finally__;
    __end__;
}


static huf_error_t
__huf_bufio_read_writer_flush(huf_bufio_read_writer_t *self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    // Flush byte buffer if full.
    if (self->byte_offset >= self->size) {
        err = huf_write(self->read_writer->writer, self->byte_rwbuf, self->size);
        __assert__(err);

        // Renew byte offset
        self->have_been_written += self->size;
        self->byte_offset = 0;
    }

    __finally__;
    __end__;
}


huf_error_t
huf_bufio_write(huf_bufio_read_writer_t *self, const void *buf, size_t len)
{
    __try__;

    huf_error_t err;

    void *buf_ptr;
    size_t diff;

    __argument__(self);
    __argument__(buf);

    buf_ptr = *buf;

    err = __huf_bufio_read_writer_flush(self);
    __assert__(err);

    diff = self->size - self->byte_offset;

    // If there is a data in buffer, then copy data from specified buffer
    // and dump it to writer.
    if (self->byte_offset && len >= diff) {
        memcpy(self->byte_rwbuf + self->byte_offset, buf_ptr, diff);

        // Next call could fail, so increase offset of the buffer.
        self->byte_offset = self-size;

        err = huf_write(self->read_writer->writer, self->byte_rwbuf, self->size);
        __assert__(err);

        buf_ptr += diff;
        len -= diff;

        self->byte_offset = 0;
        self->have_been_written += self->size;
    }

    // All other data dump to writer withot copying into buffer.
    while (len >= self->size) {
        err = huf_write(self->read_writer->writer, buf_ptr, self->size);
        __assert__(err);

        buf_ptr += self->size;
        len -= self->size;

        self->have_been_written += self->size;
    }

    memcpy(self->byte_rwbuf, buf_ptr, len);
    self->byte_offset = len;

    __finally__;
    __end__;
}


huf_error_t
huf_bufio_write_uint8(huf_bufio_read_writer_t *self, uint8_t byte)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    // Flush buffer if full.
    err = __huf_bufio_read_writer_flush(self);
    __assert__(err);

    // Put byte into the buffer.
    self->byte_offset++;
    self->byte_rwbuf[self->byte_offset] = byte;

    __finally__;
    __end__;
}
