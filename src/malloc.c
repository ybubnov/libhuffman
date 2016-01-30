#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/sys.h"


// Allocate the memory block of the specified size.
huf_error_t
huf_malloc(void** ptr, size_t size, size_t num)
{
    __try__;

    __argument__(ptr);

    *ptr = calloc(num, size);
    if (!*ptr) {
        __raise__(HUF_ERROR_MEMORY_ALLOCATION);
    }

    __finally__;
    __end__;
}
