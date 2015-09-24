#ifndef INCLUDE_huffman_histogram_h__
#define INCLUDE_huffman_histogram_h__

#include "huffman/common.h"
#include "huffman/errors.h"


typedef struct __huf_histogram {
    // Frequency chart.
    uint64_t *frequencies;

    // Size in bytes of the each element. So value for the
    // very next value will be read from incoming data with a
    // shift in iota bytes. That value should be less than 8,
    // which means only 64 bit elements are allowed.
    size_t iota;

    // Count of the frequency histogram elements. Each
    // element length is the count of iota bytes.
    size_t length;

    // First position of non-zero value. This is could
    // be usefull while iterating through frequencies to
    // skip empty values.
    size_t start;
} huf_histogram_t;


huf_error_t
huf_histogram_init(huf_histogram_t **self, size_t iota, size_t length);


huf_error_t
huf_histogram_free(huf_histogram_t **self);


huf_error_t
huf_histogram_reset(huf_histogram_t *self);


huf_error_t
huf_histogram_populate(huf_histogram_t *self, void *buf, size_t len);


#endif // INCLUDE_huffman_histogram_h__
