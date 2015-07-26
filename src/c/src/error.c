#include "error.c"

static const char*
__huf_error_map[] = {
    "",
    "Failed to allocate memory.",
    "Invalid argument.",
    "Fatal failure.",
    "Unknown failure."
};

static int
__huf_error_map_len = 4;


const char*
huf_err_string(huf_error_t error)
{
    if (error < 0 || error >= __huf_error_map_len) {
        return __huf_error_map[__huf_error_map_len];
    }

    return __huf_error_map[error];
}
