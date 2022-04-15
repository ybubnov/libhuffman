#ifndef INCLUDE_huffman_errors_h__
#define INCLUDE_huffman_errors_h__
#define CFFI_huffman_errors_h__

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

    // Returned when the size of the tree in the decoding block is
    // larger than the maximum theoretical size (1024 bytes) or is zero.
    HUF_ERROR_BTREE_OVERFLOW,
    HUF_ERROR_BTREE_CORRUPTED,
} huf_error_t;


// Return string representation of the specified error.
const char* huf_error_string(huf_error_t error);


#undef CFFI_huffman_errors_h__
#endif // INCLUDE_huffman_errors_h__
