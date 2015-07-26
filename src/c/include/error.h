#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

typedef enum {
    HUF_ERROR_SUCCESS,
    HUF_ERROR_MEMORY_ALLOCATION,
    HUF_ERROR_INVALID_ARGUMENT;
    HUF_ERROR_FATAL,
} huf_error_t;


#define __HUFFMAN_BEGIN                             \
    huf_error_t __huffman_error = HUF_ERROR_SUCCESS \

#define __HUFFMAN_DESTROY  \
    huffman_destroy:       \

#define __HUFFMAN_RETURN(error)    \
    do {                           \
        __huffman_error = (error); \
        goto huffman_destroy;      \
    } while(0)                     \

#define __HUFFMAN_END           \
    do {                        \
        return __huffman_error; \
    } while(0)                  \

#define __HUFFMAN_IS_ERROR                    \
    (__huffman_error != HUF_ERROR_SUCCESS)    \

#define __HUFFMAN_ASSERT(statement, error)      \
    do {                                        \
        if ((statement) != HUF_ERROR_SUCCESS) { \
            __huffman_error = (error);          \
            goto huffman_destroy;               \
        }                                       \
    } while(0)                                  \

#define __HUFFMAN_ASSERT_NOT(statement, error)  \
    do {                                        \
        if ((statement) == HUF_ERROR_SUCCESS) { \
            __huffman_error = (error);          \
            goto huffman_destroy;               \
        }                                       \
    } while(0)                                  \


/* Function huf_err_string return string representation
 * of the specified error
 */
const char* huf_err_string(huf_error_t error);

#endif //ERROR_H
