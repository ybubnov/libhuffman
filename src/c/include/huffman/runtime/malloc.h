#ifndef RUNTIME_MALLOC_H
#define RUNTIME_MALLOC_H

#include <stdint.h>

#include <huffman/errors.h>


huf_error_t
huf_malloc(void** ptr, size_t size, size_t num);


#endif // RUNTIME_MALLOC_H
