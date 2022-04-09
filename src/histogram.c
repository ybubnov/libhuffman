#include <string.h>

#include <huffman/histogram.h>
#include <huffman/malloc.h>
#include <huffman/sys.h>


// Initialize a new instance of the frequency histogram.
huf_error_t
huf_histogram_init(
        huf_histogram_t **self,
        size_t iota, size_t length)
{
    routine_m();

    routine_param_m(self);
    routine_param_m(iota);
    routine_param_m(length);

    huf_error_t err = huf_malloc(void_pptr_m(self),
            sizeof(huf_histogram_t), 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    huf_histogram_t *self_ptr = *self;

    err = huf_malloc(void_pptr_m(&self_ptr->frequencies),
            sizeof(uint64_t), length);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    self_ptr->iota = iota;
    self_ptr->length = length;
    self_ptr->start = -1;

    routine_yield_m();
}


// Release memory occupied by the frequency histogram.
huf_error_t
huf_histogram_free(huf_histogram_t **self)
{
    routine_m();
    routine_param_m(self);

    huf_histogram_t *self_ptr = *self;

    free(self_ptr->frequencies);
    free(self_ptr);

    *self = NULL;

    routine_yield_m();
}


// Reset all collected statistics.
huf_error_t
huf_histogram_reset(huf_histogram_t *self)
{
    routine_m();
    routine_param_m(self);

    memset(self->frequencies, 0, sizeof(uint64_t) * self->length);
    self->start = -1;

    routine_yield_m();
}


// Increase the appropriate element of the frequencies
// chart by one if the element was found in the specified
// buffer.
huf_error_t
huf_histogram_populate(
        huf_histogram_t *self,
        void *buf, size_t len)
{
    routine_m();

    uint8_t *buf_ptr = buf;
    uint8_t *buf_end = buf_ptr + len;

    routine_param_m(self);
    routine_param_m(buf);

    // Calculate frequencies of the symbols.
    while (buf_ptr + self->iota <= buf_end) {
        // Reset the destination variable.
        uint64_t element = 0;

        // Read the next element into 64 bit variable.
        memcpy(&element, buf_ptr, self->iota);

        // Shift buffer offset.
        buf_ptr += self->iota;

        self->frequencies[element] += 1;

        if (element < self->start || self->start == -1) {
            self->start = element;
        }
    }

    routine_yield_m();
}
