#ifndef INCLUDE_huffman_errors_h__
#define INCLUDE_huffman_errors_h__


typedef enum {
    HUF_ERROR_SUCCESS,
    HUF_ERROR_MEMORY_ALLOCATION,
    HUF_ERROR_INVALID_ARGUMENT,
    HUF_ERROR_READ_WRITE,
    HUF_ERROR_FATAL,
} huf_error_t;


/* Function huf_err_string return string representation
 * of the specified error
 */
const char* huf_err_string(huf_error_t error);


#endif // INCLUDE_huffman_errors_h__
