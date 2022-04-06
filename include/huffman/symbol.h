#ifndef INCLUDE_huffman_symbol_h__
#define INCLUDE_huffman_symbol_h__

#include <huffman/common.h>
#include <huffman/errors.h>

#define CFFI_huffman_symbol_h__

// An element of the symbol mapping.
typedef struct __huf_symbol_mapping_element {
    // Length of the symbol coding.
    size_t length;

    // Binary symbol coding.
    uint8_t *coding;
} huf_symbol_mapping_element_t;


// Initialize a new instance of the symbol
// mapping element.
huf_error_t
huf_symbol_mapping_element_init(
        huf_symbol_mapping_element_t **self,
        const uint8_t *coding,
        size_t length);


// Release memory occupied by the symbol
// mapping element.
huf_error_t
huf_symbol_mapping_element_free(
        huf_symbol_mapping_element_t **self);


// A symbol mapping.
typedef struct __huf_symbol_mapping {
    // Count of the elements in the mapping.
    size_t length;

    // Array of the symbols encodings.
    huf_symbol_mapping_element_t **symbols;
} huf_symbol_mapping_t;


// Initialize a new instance of the symbol mapping.
huf_error_t
huf_symbol_mapping_init(
        huf_symbol_mapping_t **self,
        size_t length);


// Release memory occupied by the symbol mapping.
huf_error_t
huf_symbol_mapping_free(
        huf_symbol_mapping_t **self);


// Insert an element into the symbol mapping by
// the specified position.
huf_error_t
huf_symbol_mapping_insert(
        huf_symbol_mapping_t *self,
        size_t position,
        huf_symbol_mapping_element_t *element);


// Retrieve values of the symbol mapping element
// by the specified position.
huf_error_t
huf_symbol_mapping_get(
        huf_symbol_mapping_t *self,
        size_t position,
        huf_symbol_mapping_element_t **element);


// Reset the memory occupied by the symbol mapping.
huf_error_t
huf_symbol_mapping_reset(
        huf_symbol_mapping_t *self);


#undef CFFI_huffman_symbol_h__
#endif // INCLUDE_huffman_symbol_h__
