#include "runtime.h"
#include "internal.h"
#include "error.h"
#include "core.h"


huf_error_t
huf_init(huf_ctx_t* hctx, int ifd, int ofd, uint64_t length)
{
    __try__;

    huf_error_r err;

    __argument__(hctx);

    memset(hctx, 0, sizeof(huf_ctx_t));

    err = huf_alloc(&hctx->leaves, sizeof(huf_node_t*), 256);
    __assert__(err);

    __finally__;
    __end__;
}
