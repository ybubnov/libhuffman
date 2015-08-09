#ifndef INCLUDE_huffman_io_h__
#define INCLUDE_huffman_io_h__

#include "huffman/common.h"
#include "huffman/errors.h"


/* huf_writer_t is writer abstraction.
 */
typedef int huf_writer_t;

/* huf_reader_r is reader abstraction.
 */
typedef int huf_reader_t;


/* huf_read_writer_t stores reader and writer.
 */
typedef struct __huf_read_writer {
    huf_reader_t reader;
    huf_writer_t writer;
    uint64_t len;
} huf_read_writer_t;


/* huf_write writes count bytes from buf array.
 */
huf_error_t
huf_write(huf_writer_t writer, const void *buf, size_t count);

/* huf_reader reads count bytes to buf array and returs number of bytes read.
 */
huf_error_t
huf_read(huf_reader_t reader, void *buf, size_t *count);


#endif // INCLUDE_huffman_io_h__
