#include "huffman/config.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


// Initialize a new instance of the configuration.
huf_error_t
huf_config_init(huf_config_t **self)
{
    routine_m();
    routine_param_m(self);

    huf_error_t err = huf_malloc(void_pptr_m(self), sizeof(huf_config_t), 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    routine_yield_m();
}


// Release memory occupied by the configuration.
huf_error_t
huf_config_free(huf_config_t **self)
{
    routine_m();
    routine_param_m(self);

    free(*self);
    *self = NULL;

    routine_yield_m();
}
