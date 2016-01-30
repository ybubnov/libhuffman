#ifndef INCLUDE_huffman_errors_h__
#define INCLUDE_huffman_errors_h__


// Enumeration of the possible errors.
typedef enum {
    // The call completed successfully.
    HUF_ERROR_SUCCESS,

    // Failed to allocate the memory block.
    HUF_ERROR_MEMORY_ALLOCATION,

    // The specified argument is invalid,
    // e.g. the pointer is equal to nil.
    HUF_ERROR_INVALID_ARGUMENT,

    // The failure caused by input/output error.
    HUF_ERROR_READ_WRITE,

    // Failure caused by Unrecoverable error.
    HUF_ERROR_FATAL,
} huf_error_t;


// Return string representation of the specified error.
const char* huf_error_string(huf_error_t error);


#endif // INCLUDE_huffman_errors_h__
