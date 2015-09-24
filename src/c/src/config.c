#include "huffman/config.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


huf_error_t
huf_encoder_config_init(huf_encoder_config_t **self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    err = huf_malloc((void**) self, sizeof(huf_encoder_config_t), 1);
    __assert__(err);

    __finally__;
    __end__;
}

huf_error_t
huf_encoder_config_free(huf_encoder_config_t **self)
{
    __try__;

    __argument__(self);

    free(*self);
    *self = NULL;

    __finally__;
    __end__;
}


huf_error_t
huf_decoder_config_init(huf_decoder_config_t **self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    err = huf_malloc((void**) self, sizeof(huf_decoder_config_t), 1);
    __assert__(err);

    __finally__;
    __end__;
}

huf_error_t
huf_decoder_config_free(huf_decoder_config_t **self)
{
    __try__;

    __argument__(self);

    free(*self);
    *self = NULL;

    __finally__;
    __end__;
}
