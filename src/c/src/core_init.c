#include "rt.h"
#include "error.h"
#include "core.h"


huf_error_t
huf_init(huf_ctx_t* hctx, int ifd, int ofd, uint64_t length)
{
    __TRY__;

    memset(hctx, 0, sizeof(huf_ctx_t));

    hctx->leaves = calloc(256, sizeof(huf_node_t*));
    if (!hctx->leaves) {
        __RAISE__(HUF_ERROR_MEMORY_ALLOCATION);
    }

    __FINALLY__;
    __END__;
}
