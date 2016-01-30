#include "huffman/config.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


// Initialize a new instance of the configuration.
huf_error_t
huf_config_init(huf_config_t **self)
{
    __try__;
    __argument__(self);

    huf_error_t err = huf_malloc(void_pptr_m(self),
            sizeof(huf_config_t), 1);
    __assert__(err);

    __finally__;
    __end__;
}


// Release memory occupied by the configuration.
huf_error_t
huf_config_free(huf_config_t **self)
{
    __try__;
    __argument__(self);

    free(*self);
    *self = NULL;

    __finally__;
    __end__;
}
