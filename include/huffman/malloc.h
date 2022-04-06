#ifndef INCLUDE_huffman_malloc_h__
#define INCLUDE_huffman_malloc_h__

#include "huffman/common.h"
#include "huffman/errors.h"

#define CFFI_huffman_malloc_h__

// Allocate the memory block of the specified size.
huf_error_t
huf_malloc(void** ptr, size_t size, size_t num);


#undef CFFI_huffman_malloc_h__
#endif // INCLUDE_huffman_malloc_h__
