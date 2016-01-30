#include <string.h>

#include "huffman/symbol.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


// Initialize a new instance of the symbol
// mapping element.
huf_error_t
huf_symbol_mapping_element_init(
        huf_symbol_mapping_element_t **self,
        const uint8_t *coding,
        size_t length)
{
    __try__;

    huf_error_t err;
    huf_symbol_mapping_element_t *self_ptr;

    __argument__(self);
    __argument__(coding);

    err = huf_malloc(void_pptr_m(self),
            sizeof(huf_symbol_mapping_element_t), 1);
    __assert__(err);

    self_ptr = *self;

    err = huf_malloc(void_pptr_m(&self_ptr->coding),
            sizeof(uint8_t), length + 1);
    __assert__(err);

    self_ptr->length = length;
    memcpy(self_ptr->coding, coding, length);

    __finally__;
    __end__;
}


// Release memory occupied by the symbol
// mapping element.
huf_error_t
huf_symbol_mapping_element_free(
        huf_symbol_mapping_element_t **self)
{
    __try__;

    huf_symbol_mapping_element_t *self_ptr;

    __argument__(self);

    self_ptr = *self;

    free(self_ptr->coding);
    free(self_ptr);

    *self = NULL;

    __finally__;
    __end__;
}


// Initialize a new instance of the symbol mapping.
huf_error_t
huf_symbol_mapping_init(
        huf_symbol_mapping_t **self,
        size_t length)
{
    __try__;

    huf_error_t err;
    huf_symbol_mapping_t *self_ptr;

    __argument__(self);

    err = huf_malloc(void_pptr_m(self),
            sizeof(huf_symbol_mapping_t), 1);
    __assert__(err);

    self_ptr = *self;

    err = huf_malloc(void_pptr_m(&self_ptr->symbols),
            sizeof(huf_symbol_mapping_element_t*), length);
    __assert__(err);

    self_ptr->length = length;

    __finally__;
    __end__;
}


// Reset memory occipied by the symbol mapping.
static huf_error_t
__huf_symbol_mapping_free(
        huf_symbol_mapping_t *self)
{
    __try__;

    huf_error_t err;
    huf_symbol_mapping_element_t *element;

    size_t index;

    __argument__(self);

    for (index = 0; index < self->length; index++) {
        element = self->symbols[index];

        if (!element) {
            continue;
        }

        err = huf_symbol_mapping_element_free(&element);
        __assert__(err);

        self->symbols[index] = NULL;
    }

    __finally__;
    __end__;
}


// Release memory occupied by the symbol mapping.
huf_error_t
huf_symbol_mapping_free(
        huf_symbol_mapping_t **self)
{
    __try__;

    huf_error_t err;
    huf_symbol_mapping_t *self_ptr;

    __argument__(self);

    self_ptr = *self;

    err = __huf_symbol_mapping_free(self_ptr);
    __assert__(err);

    free(self_ptr->symbols);
    free(self_ptr);

    *self = NULL;

    __finally__;
    __end__;
}


// Insert an element into the symbol mapping by
// the specified position.
huf_error_t
huf_symbol_mapping_insert(
        huf_symbol_mapping_t *self,
        size_t position,
        huf_symbol_mapping_element_t *element)
{
    __try__;

    huf_error_t err;
    huf_symbol_mapping_element_t *previous_element;

    __argument__(self);
    __argument__(element);

    __inrange__(position, 0, self->length - 1);

    previous_element = self->symbols[position];

    if (previous_element) {
        err = huf_symbol_mapping_element_free(
                &previous_element);
        __assert__(err);
    }

    self->symbols[position] = element;

    __finally__;
    __end__;
}


// Retrieve values of the symbol mapping element
// by the specified position.
huf_error_t
huf_symbol_mapping_get(
        huf_symbol_mapping_t *self,
        size_t position,
        huf_symbol_mapping_element_t **element)
{
    __try__;

    __argument__(self);
    __argument__(element);

    __inrange__(position, 0, self->length - 1);

    *element = self->symbols[position];

    __finally__;
    __end__;
}


// Reset the memory occupied by the symbol mapping.
huf_error_t
huf_symbol_mapping_reset(huf_symbol_mapping_t *self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    err = __huf_symbol_mapping_free(self);
    __assert__(err);

    __finally__;
    __end__;
}
