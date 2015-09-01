#ifndef INCLUDE_huffman_symbol_h__
#define INCLUDE_huffman_symbol_h__

#include "huffman/common.h"
#include "huffman/errors.h"


typedef struct __huf_symbol_mapping_element {
    // Length of the symbol coding.
    size_t length;

    // Binary symbol coding.
    uint8_t *coding;
} huf_symbol_mapping_element_t;


huf_error_t
huf_symbol_mapping_element_init(huf_symbol_mapping_element_t **self, const uint8_t *coding, size_t length);


huf_error_t
huf_symbol_mapping_element_free(huf_symbol_mapping_element_t **self);


typedef struct __huf_symbol_mapping {
    // Count of the elements in the mapping.
    size_t length;

    // Array of the symbols encodings.
    huf_symbol_mapping_element_t **symbols;
} huf_symbol_mapping_t;


huf_error_t
huf_symbol_mapping_init(huf_symbol_mapping_t **self, size_t length);


huf_error_t
huf_symbol_mapping_free(huf_symbol_mapping_t **self);


huf_error_t
huf_symbol_mapping_insert(huf_symbol_mapping_t *self, size_t position, huf_symbol_mapping_element_t *element);


huf_error_t
huf_symbol_mapping_get(huf_symbol_mapping_t *self, size_t position, huf_symbol_mapping_element_t **element);


huf_error_t
huf_symbol_mapping_reset(huf_symbol_mapping_t *self);


#endif // INCLUDE_huffman_symbol_h__
