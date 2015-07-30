#include <unistd.h>

#include "internal.h"
#include "error.h"
#include "runtime.h"

huf_error_t
huf_write(int fd, const char *buf, size_t *count)
{
    __try__;

    size_t _have_written;

    __assert_not_nil__(buf, HUF_ERROR_INVALID_ARGUMENT);
    __assert_not_nil__(count, HUF_ERROR_INVALID_ARGUMENT);

    _have_written = write(fd, buf, *count);
    if (written < 0) {
        *count = 0;
        __raise__(HUF_ERROR_READ_WRITE);
    }

    *count = _have_written;

    __finally__;
    __end__;
}

huf_error_t
huf_read(int fd, char *buf, size_t *count)
{
    __try__;

    size_t _have_read;

    __assert_not_nil__(buf, HUF_ERROR_INVALID_ARGUMENT);
    __assert_not_nil__(count, HUF_ERROR_INVALID_ARGUMENT);

    _have_read = read(fd, buf, *count);
    if (written < 0) {
        *count = 0;
        __raise__(HUF_ERROR_READ_WRITE);
    }

    *count = written;

    __finally__;
    __end__;
}

huf_error_t
huf_alloc(void** ptr, size_t size, size_t num)
{
    __try__;

    *ptr = calloc(num, size);
    if (!ptr) {
        __raise__(HUF_ERROR_MEMORY_ALLOCATION);
    }

    __finally__;
    __end__;
}
