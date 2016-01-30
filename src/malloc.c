#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/sys.h"


// Allocate the memory block of the specified size.
huf_error_t
huf_malloc(void** ptr, size_t size, size_t num)
{
    routine_m();
    routine_param_m(ptr);

    *ptr = calloc(num, size);
    if (!*ptr) {
        routine_error_m(HUF_ERROR_MEMORY_ALLOCATION);
    }

    routine_yield_m();
}
