#include "huffman/config.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


huf_error_t
huf_config_init(huf_config_t **self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    err = huf_malloc((void**) self, sizeof(huf_config_t), 1);
    __assert__(err);

    __finally__;
    __end__;
}

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
