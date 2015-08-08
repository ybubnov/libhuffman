#include <string.h>

#include <huffman/huffman.h>
#include <huffman/runtime/malloc.h>
#include <huffman/runtime/sys.h>


huf_error_t
huf_init(huf_archiver_t **self)
{
    __try__;

    huf_archiver_t *self_pointer;
    huf_error_t err;

    __argument__(self);

    self_pointer = *self;

    err = huf_malloc((void**) self, sizeof(huf_archiver_t), 1);
    __assert__(err);

    err = huf_malloc((void**) &self_pointer->leaves, sizeof(huf_node_t*), 256);
    __assert__(err);

    __finally__;
    __end__;
}
