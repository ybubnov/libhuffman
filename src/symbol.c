#include <string.h>

#include <huffman/symbol.h>
#include <huffman/malloc.h>
#include <huffman/sys.h>


// Initialize a new instance of the symbol
// mapping element.
huf_error_t
huf_symbol_mapping_element_init(
        huf_symbol_mapping_element_t **self,
        const uint8_t *coding,
        size_t length)
{
    routine_m();

    huf_error_t err;
    huf_symbol_mapping_element_t *self_ptr;

    routine_param_m(self);
    routine_param_m(coding);

    err = huf_malloc(void_pptr_m(self),
            sizeof(huf_symbol_mapping_element_t), 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    self_ptr = *self;

    err = huf_malloc(void_pptr_m(&self_ptr->coding),
            sizeof(uint8_t), length + 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    self_ptr->length = length;
    memcpy(self_ptr->coding, coding, length);

    routine_yield_m();
}


// Release memory occupied by the symbol
// mapping element.
huf_error_t
huf_symbol_mapping_element_free(
        huf_symbol_mapping_element_t **self)
{
    routine_m();
    routine_param_m(self);

    huf_symbol_mapping_element_t *self_ptr = *self;

    free(self_ptr->coding);
    free(self_ptr);

    *self = NULL;

    routine_yield_m();
}


// Initialize a new instance of the symbol mapping.
huf_error_t
huf_symbol_mapping_init(
        huf_symbol_mapping_t **self,
        size_t length)
{
    routine_m();

    huf_error_t err;
    huf_symbol_mapping_t *self_ptr;

    routine_param_m(self);

    err = huf_malloc(void_pptr_m(self),
            sizeof(huf_symbol_mapping_t), 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    self_ptr = *self;

    err = huf_malloc(void_pptr_m(&self_ptr->symbols),
            sizeof(huf_symbol_mapping_element_t*), length);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    self_ptr->length = length;

    routine_yield_m();
}


// Reset memory occipied by the symbol mapping.
static huf_error_t
__huf_symbol_mapping_free(
        huf_symbol_mapping_t *self)
{
    routine_m();

    huf_error_t err;
    huf_symbol_mapping_element_t *element;

    size_t index;

    routine_param_m(self);

    for (index = 0; index < self->length; index++) {
        element = self->symbols[index];

        if (!element) {
            continue;
        }

        err = huf_symbol_mapping_element_free(&element);
        if (err != HUF_ERROR_SUCCESS) {
            routine_error_m(err);
        }

        self->symbols[index] = NULL;
    }

    routine_yield_m();
}


// Release memory occupied by the symbol mapping.
huf_error_t
huf_symbol_mapping_free(
        huf_symbol_mapping_t **self)
{
    routine_m();
    routine_param_m(self);

    huf_symbol_mapping_t *self_ptr = *self;

    huf_error_t err = __huf_symbol_mapping_free(self_ptr);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    free(self_ptr->symbols);
    free(self_ptr);

    *self = NULL;

    routine_yield_m();
}


// Insert an element into the symbol mapping by
// the specified position.
huf_error_t
huf_symbol_mapping_insert(
        huf_symbol_mapping_t *self,
        size_t position,
        huf_symbol_mapping_element_t *element)
{
    routine_m();

    huf_error_t err;
    huf_symbol_mapping_element_t *previous_element;

    routine_param_m(self);
    routine_param_m(element);

    routine_inrange_m(position, 0, self->length - 1);

    previous_element = self->symbols[position];

    if (previous_element) {
        err = huf_symbol_mapping_element_free(
                &previous_element);
        if (err != HUF_ERROR_SUCCESS) {
            routine_error_m(err);
        }
    }

    self->symbols[position] = element;

    routine_yield_m();
}


// Retrieve values of the symbol mapping element
// by the specified position.
huf_error_t
huf_symbol_mapping_get(
        huf_symbol_mapping_t *self,
        size_t position,
        huf_symbol_mapping_element_t **element)
{
    routine_m();

    routine_param_m(self);
    routine_param_m(element);

    routine_inrange_m(position, 0, self->length - 1);
    *element = self->symbols[position];
    printf("!!! %p %p\n", element, self->symbols[position]);

    routine_yield_m();
}


// Reset the memory occupied by the symbol mapping.
huf_error_t
huf_symbol_mapping_reset(huf_symbol_mapping_t *self)
{
    routine_m();
    routine_param_m(self);

    huf_error_t err = __huf_symbol_mapping_free(self);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    routine_yield_m();
}
