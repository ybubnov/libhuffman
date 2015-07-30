#include "rt.h"
#include "error.h"
#include "core.h"


huf_error_t
huf_init(huf_ctx_t* hctx, int ifd, int ofd, uint64_t length)
{
    __try__;

    __assert_not_nil__(hctx, HUF_INVALID_ARGUMENT);

    memset(hctx, 0, sizeof(huf_ctx_t));

    hctx->leaves = calloc(256, sizeof(huf_node_t*));
    __assert_nil__(hctx->leaves, HUF_ERROR_MEMORY_ALLOCATION);

    __finally__;
    __end__;
}
