#include <string.h>

#include "runtime.h"
#include "internal.h"
#include "core.h"


huf_error_t
huf_init(huf_ctx_t* self)
{
    __try__;

    huf_error_t err;

    __argument__(self);

    memset(self, 0, sizeof(huf_ctx_t));

    err = huf_alloc((void**) &self->leaves, sizeof(huf_node_t*), 256);
    __assert__(err);

    __finally__;
    __end__;
}
