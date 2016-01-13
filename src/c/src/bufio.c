#include <string.h>

#include "huffman/bufio.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


huf_error_t
huf_bufio_read_writer_init(huf_bufio_read_writer_t **self, huf_read_writer_t *read_writer, size_t capacity)
{
    __try__;

    huf_bufio_read_writer_t *self_ptr = NULL;
    huf_error_t err;

    __argument__(self);

    err = huf_malloc((void**) &self_ptr, sizeof(huf_bufio_read_writer_t), 1);
    __assert__(err);

    *self = self_ptr;

    err = huf_malloc((void**) &self_ptr->bytes, sizeof(uint8_t), capacity);
    __assert__(err);

    // If zero value provided for capacity, then use
    // 64 KiB buffer by default.
    if (!capacity) {
        capacity = __64KIB_BUFFER;
    }

    self_ptr->capacity = capacity;
    self_ptr->read_writer = read_writer;

    __finally__;
    __end__;
}


huf_error_t
huf_bufio_read_writer_free(huf_bufio_read_writer_t **self)
{
    __try__;

    huf_bufio_read_writer_t *self_ptr;

    __argument__(self);

    self_ptr = *self;

    free(self_ptr->bytes);
    free(self_ptr);

    *self = NULL;

    __finally__;
    __end__;
}


huf_error_t
huf_bufio_read_writer_flush(huf_bufio_read_writer_t *self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    // If the buffer is empty, there is nothing to do then.
    if (self->length <= 0) {
        __success__;
    }

    printf("FLUSHING BYTES = %lld\n", (long long) self->length);

    err = huf_write(self->read_writer->writer, self->bytes, self->length);
    __assert__(err);

    self->length = 0;
    self->have_been_processed += self->length;

    __finally__;
    __end__;
}


static huf_error_t
__huf_bufio_read_writer_flush(huf_bufio_read_writer_t *self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    /*printf("FLUSH LENGTH = %lld\n", (long long) self->length);*/

    // Flush buffer if it is full.
    if (self->length >= self->capacity) {
        err = huf_write(self->read_writer->writer, self->bytes, self->length);
        __assert__(err);

        // Renew byte length
        self->have_been_processed += self->length;
        self->length = 0;
    }

    __finally__;
    __end__;
}


huf_error_t
huf_bufio_write(huf_bufio_read_writer_t *self, const void *buf, size_t len)
{
    __try__;

    huf_error_t err;

    const uint8_t *buf_ptr;
    size_t available_to_write;

    __argument__(self);
    __argument__(buf);

    buf_ptr = buf;

    err = __huf_bufio_read_writer_flush(self);
    __assert__(err);

    available_to_write = self->capacity - self->length;

    // If there is a data in buffer, then copy data from specified buffer
    // and dump it to writer.
    if (self->length && len >= available_to_write) {
        memcpy(self->bytes + self->length, buf_ptr, available_to_write);

        // Next call could fail, so increase length of the buffer.
        self->length = self->capacity;

        err = huf_write(self->read_writer->writer, self->bytes, self->capacity);
        __assert__(err);

        buf_ptr += available_to_write;
        len -= available_to_write;

        self->length = 0;
        self->have_been_processed += self->capacity;
    }

    // All other data dump to writer withot copying into buffer.
    while (len >= self->capacity) {
        err = huf_write(self->read_writer->writer, buf_ptr, self->capacity);
        __assert__(err);

        buf_ptr += self->capacity;
        len -= self->capacity;

        self->have_been_processed += self->capacity;
    }

    memcpy(self->bytes + self->length, buf_ptr, len);

    self->length += len;
    self->have_been_processed += len;

    __finally__;
    __end__;
}


huf_error_t
huf_bufio_read_uint8(huf_bufio_read_writer_t *self, uint8_t *byte)
{
    __try__;

    huf_error_t err;

    __argument__(self);
    __argument__(byte);

    err = huf_bufio_read(self, byte, sizeof(*byte));
    __assert__(err);

    __finally__;
    __end__;
}


huf_error_t
huf_bufio_write_uint8(huf_bufio_read_writer_t *self, uint8_t byte)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    // Flush buffer if it is full.
    err = __huf_bufio_read_writer_flush(self);
    __assert__(err);

    // Put byte into the buffer.
    self->bytes[self->length] = byte;
    self->length++;

    __finally__;
    __end__;
}


huf_error_t
huf_bufio_read(huf_bufio_read_writer_t *self, void *buf, size_t len)
{
    __try__;

    huf_error_t err;

    uint8_t *buf_ptr = buf;

    size_t len_copy;
    size_t available_to_read;
    size_t bytes_to_copy;

    __argument__(self);
    __argument__(buf);

    len_copy = len;

    // Get count of available in buffer bytes.
    available_to_read = self->length - self->offset;

    // If there is a data in buffer, then copy it to destination buffer.
    if (available_to_read > 0 && len > 0) {
        bytes_to_copy = available_to_read;

        if (available_to_read > len) {
            bytes_to_copy = len;
        }

        // Copy as much bytes as available.
        memcpy(buf_ptr, self->bytes + self->offset, bytes_to_copy);

        // Recalculate the offset and update buffer pointer.
        self->offset += bytes_to_copy;
        buf_ptr += bytes_to_copy;

        // Update length value, so we could continue to fill the bytes.
        len -= bytes_to_copy;
    }

    // If request length was satisfied, then simply exit.
    if (len <= 0) {
        // Leave the function with HUF_ERROR_SUCCESS.
        __success__;
    }

    // If there is still data required to read and it is larger than
    // buffer capacity, then just read it directly to the destination buffer.
    if (len >= self->capacity) {
        err = huf_read(self->read_writer->reader, buf_ptr, &len);
        __assert__(err);

        self->length = 0;
        self->offset = 0;

        // Leave the function with HUF_ERROR_SUCCESS.
        __success__;
    }

    // Try to fill available buffer.
    self->length = self->capacity;

    // In case when buffer size is larger that requested data.
    // Read bytes into buffer first and then copy bytes to destination.
    err = huf_read(self->read_writer->reader, self->bytes, &self->length);
    __assert__(err);

    memcpy(buf_ptr, self->bytes, len);
    self->offset = len;

    __finally__;

    // Update integral counter.
    if (!__raised__) {
        self->have_been_processed += len_copy;
    }

    __end__;
}
