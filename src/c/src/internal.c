#include <unistd.h>

#include "internal.h"
#include "error.h"
#include "runtime.h"


huf_error_t
huf_write(int fd, const void *buf, size_t count)
{
    __try__;

    size_t have_written;

    __argument__(buf);

    have_written = write(fd, buf, count);
    if (have_written < 0) {
        __raise__(HUF_ERROR_READ_WRITE);
    }

    __finally__;
    __end__;
}


huf_error_t
huf_read(int fd, void *buf, size_t count)
{
    __try__;

    size_t _have_read;

    __argument__(buf);

    _have_read = read(fd, buf, count);
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

    __argument__(ptr);

    *ptr = calloc(num, size);
    if (!ptr) {
        __raise__(HUF_ERROR_MEMORY_ALLOCATION);
    }

    __finally__;
    __end__;
}
