#include <unistd.h>

#include "huffman/errors.h"
#include "huffman/io.h"
#include "huffman/sys.h"


huf_error_t
huf_write(huf_writer_t writer, const void *buf, size_t count)
{
    __try__;

    size_t have_written;

    __argument__(buf);

    have_written = write(writer, buf, count);
    if (have_written < 0) {
        __raise__(HUF_ERROR_READ_WRITE);
    }

    __finally__;
    __end__;
}


huf_error_t
huf_read(huf_reader_t reader, void *buf, size_t *count)
{
    __try__;

    size_t have_read;

    __argument__(buf);
    __argument__(count);

    have_read = read(reader, buf, *count);
    if (have_read < 0) {
        *count = 0;
        __raise__(HUF_ERROR_READ_WRITE);
    }

    *count = have_read;

    __finally__;
    __end__;
}
