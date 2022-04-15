#include "huffman/common.h"
#include "huffman/errors.h"


static const char*
__huf_error_map[] = {
    "Success",
    "Failed to allocate the requested memory block",
    "An invalid argument was specified to the function",
    "Failed on read/write operation",
    "Fatal error",
    "Block is corrupted, Huffman tree has impossible size",
    "Huffman tree is corrupted and cannot be used to decode the block",
    "Unknown error"
};


// Return string representation of the specified error.
const char*
huf_error_string(huf_error_t error)
{
    size_t huf_error_map_len = (sizeof(__huf_error_map) / sizeof(*__huf_error_map));

    // Return "Uknown failure" if the specified error
    // values is not defined in the error list.
    if (error < 0 || error >= huf_error_map_len) {
        return __huf_error_map[huf_error_map_len-1];
    }

    // Otherwise simply return the string by the
    // position in the map.
    return __huf_error_map[error];
}
