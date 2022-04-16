#include <string.h>

#include "huffman/bufio.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


#define __write_m(rw, buf, count) \
    (rw->write(rw->stream, buf, count)) \


#define __read_m(rw, buf, count) \
    (rw->read(rw->stream, buf, count)) \


// Write the first bit of the specified word into the
// bit buffer.
void
huf_bit_write(huf_bit_read_writer_t *self, uint8_t bit)
{
    self->offset -= self->offset ? 1 : 0;
    self->bits |= (bit & 1) << self->offset;
}


// Reset the content of the buffer.
void
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
        size_t capacity)
{
    routine_m();

    huf_bufio_read_writer_t *self_ptr = NULL;

    routine_param_m(self);
    routine_param_m(read_writer);

    huf_error_t err = huf_malloc(void_pptr_m(&self_ptr),
            sizeof(huf_bufio_read_writer_t), 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    *self = self_ptr;

    // If zero value provided for capacity, then use 64 KiB buffer by default.
    //if (!capacity) {
    //    capacity = HUF_64KIB_BUFFER;
    //}

    if (capacity) {
        err = huf_malloc(void_pptr_m(&self_ptr->bytes), sizeof(uint8_t), capacity);
        if (err != HUF_ERROR_SUCCESS) {
            routine_error_m(err);
        }
    }

    self_ptr->capacity = capacity;
    self_ptr->read_writer = read_writer;

    routine_yield_m();
}


// Release memory occupied by the read-write buffer.
huf_error_t
huf_bufio_read_writer_free(huf_bufio_read_writer_t **self)
{
    routine_m();
    routine_param_m(self);

    huf_bufio_read_writer_t *self_ptr = *self;

    free(self_ptr->bytes);
    free(self_ptr);

    *self = NULL;

    routine_yield_m();
}


// Flush the writer buffer.
huf_error_t
huf_bufio_read_writer_flush(huf_bufio_read_writer_t *self)
{
    routine_m();
    routine_param_m(self);

    // If the buffer is empty, there is nothing to do then.
    if (self->length == 0) {
        routine_success_m();
    }

    huf_error_t err = __write_m(self->read_writer, self->bytes, self->length);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    self->length = 0;
    self->have_been_processed += self->length;

    routine_yield_m();
}


// Flush the content of the writer buffer.
static huf_error_t
__huf_bufio_read_writer_flush(huf_bufio_read_writer_t *self)
{
    routine_m();
    routine_param_m(self);

    // Flush buffer if it is full.
    if (self->length >= self->capacity && self->length) {
        huf_error_t err = __write_m(self->read_writer, self->bytes, self->length);
        if (err != HUF_ERROR_SUCCESS) {
            routine_error_m(err);
        }

        // Renew byte length
        self->have_been_processed += self->length;
        self->length = 0;

        if (self->length) {
            routine_error_m(HUF_ERROR_READ_WRITE);
        }
    }

    routine_yield_m();
}


// Write the specified amount of bytes starting from the provided pointer into the
// writer buffer. If the buffer will be filled during the copying of bytes, it could
// be flushed.
huf_error_t
huf_bufio_write(huf_bufio_read_writer_t *self, const void *buf, size_t len)
{
    routine_m();

    routine_param_m(self);
    routine_param_m(buf);

    // Attempt to flush buffer if it's full.
    huf_error_t err = __huf_bufio_read_writer_flush(self);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    size_t available_to_write = self->capacity - self->length;

    // If there is a space left in the buffer, then copy data from specified buffer.
    if (self->capacity && len <= available_to_write) {
        memcpy(self->bytes + self->length, buf, len);

        self->length += len;
        self->have_been_processed += len;
        len = 0;
    }

    // Dump the remaining data directly to the writer without copying into buffer.
    if (len > 0) {
        // Force flush of the existing bytes in a buffer.
        err = huf_bufio_read_writer_flush(self);
        if (err != HUF_ERROR_SUCCESS) {
            routine_error_m(err);
        }

        err = __write_m(self->read_writer, buf, len);
        if (err != HUF_ERROR_SUCCESS) {
            routine_error_m(err);
        }

        self->have_been_processed += len;
    }

    routine_yield_m();
}


// Read the specified amount of bytes from the reader buffer
// starting from the provided pointer.
huf_error_t
huf_bufio_read(huf_bufio_read_writer_t *self, void *buf, size_t len)
{
    routine_m();

    huf_error_t err;
    uint8_t *buf_ptr = buf;

    routine_param_m(self);
    routine_param_m(buf);

    size_t len_copy = len;

    // Get count of available in buffer bytes.
    size_t available_to_read = self->length - self->offset;

    // If there is a data in buffer, then copy it to the destination buffer.
    if (available_to_read > 0 && len > 0) {
        size_t bytes_to_copy = available_to_read;

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

    // If requested length was satisfied, then simply exit.
    if (len == 0) {
        routine_success_m();
    }

    // If there is still data required to read and it is larger
    // than buffer capacity, then just read it directly to the
    // destination buffer.
    if (len >= self->capacity) {
        size_t rem_len = len;
        err = __read_m(self->read_writer, buf_ptr, &rem_len);
        if (err != HUF_ERROR_SUCCESS) {
            routine_error_m(err);
        }

        self->length = 0;
        self->offset = 0;

        // The read operation should succeed on requested amount of bytes
        // to read. If the request is not satisfied, return an error.
        if (rem_len < len) {
            routine_error_m(HUF_ERROR_READ_WRITE);
        }

        // Leave the routine with HUF_ERROR_SUCCESS.
        routine_success_m();
    }

    // Try to fill available buffer.
    self->length = self->capacity;

    // In case when buffer size is larger that requested
    // data. Read bytes into buffer first and then copy
    // bytes to the destination.
    err = __read_m(self->read_writer, self->bytes, &self->length);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    // There is still not enough memory to satisfy the request, exit with an error.
    if (len > self->length) {
        routine_error_m(HUF_ERROR_READ_WRITE);
    }

    memcpy(buf_ptr, self->bytes, len);
    self->offset = len;

    routine_ensure_m();

    // Update integral counter when the routine was
    // interrupted due to unexpected error.
    if (!routine_violation_m()) {
        self->have_been_processed += len_copy;
    }

    routine_defer_m();
}


// Read the 8-bits word from the reader buffer into the specified pointer.
huf_error_t
huf_bufio_read_uint8(huf_bufio_read_writer_t *self, uint8_t *byte)
{
    routine_m();
    routine_param_m(self);
    routine_param_m(byte);

    huf_error_t err = huf_bufio_read(self, byte, sizeof(uint8_t));
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    routine_yield_m();
}


// Write the specified 8-bits word into the writer buffer.
huf_error_t
huf_bufio_write_uint8(huf_bufio_read_writer_t *self, uint8_t byte)
{
    routine_m();
    routine_param_m(self);

    huf_error_t err = huf_bufio_write(self, &byte, sizeof(uint8_t));
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    routine_yield_m();
}
