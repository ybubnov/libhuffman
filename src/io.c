#include <string.h>
#include <unistd.h>

#include "huffman/io.h"
#include "huffman/sys.h"
#include "huffman/malloc.h"


huf_error_t fdwrite(void *stream, const void *buf, size_t count);
huf_error_t fdread(void *stream, void *buf, size_t *count);


huf_error_t huf_fdopen(huf_read_writer_t **self, int fd)
{
    routine_m();

    huf_error_t err;
    huf_read_writer_t *self_ptr;

    routine_param_m(self);

    err = huf_malloc(void_pptr_m(self), sizeof(huf_read_writer_t), 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    self_ptr = *self;
    self_ptr->stream = (void*)(&fd);
    self_ptr->read = fdread;
    self_ptr->write = fdwrite;

    routine_yield_m();
}


huf_error_t fdwrite(void *stream, const void *buf, size_t count)
{
    size_t have_written = write(*(int*)stream, buf, count);
    if (have_written < 0) {
        return HUF_ERROR_READ_WRITE;
    }
    return HUF_ERROR_SUCCESS;
}


huf_error_t fdread(void *stream, void *buf, size_t *count)
{
    size_t have_read = read(*(int*)stream, buf, *count);
    if (have_read < 0) {
        *count = 0;
        return HUF_ERROR_READ_WRITE;
    }
    
    *count = have_read;
    return HUF_ERROR_SUCCESS;
}


huf_error_t huf_fdclose(huf_read_writer_t **self)
{
    routine_m();
    routine_param_m(self);

    huf_read_writer_t *self_ptr = *self;
    free(self_ptr);
    *self = NULL;

    routine_yield_m();
}


typedef struct __huf_memstream {
    void **buf;
    size_t len;
    size_t cap;
} huf_membuf_t;


huf_error_t memwrite(void *stream, const void *buf, size_t count)
{
    huf_membuf_t *mem = (huf_membuf_t*)stream;

    if (mem->cap > mem->len + count) {
        memcpy(*(mem->buf) + mem->len, buf, count);
        mem->len += count;
        return HUF_ERROR_SUCCESS;
    }

    return HUF_ERROR_MEMORY_ALLOCATION;
}


huf_error_t memread(void *stream, void *buf, size_t *count)
{
    huf_membuf_t *mem = (huf_membuf_t*)stream;
    size_t num_copy = *count;

    if (num_copy > mem->len) {
        num_copy = mem->len;
        *count = num_copy;
    }

    if (num_copy > 0) {
        memcpy(buf, *mem->buf, num_copy);
        mem->len -= num_copy;
    }

    return HUF_ERROR_SUCCESS;
}


// Return the length of the membuf into the len argument.
huf_error_t huf_memlen(huf_read_writer_t *self, size_t *len)
{
    routine_m();

    routine_param_m(self);
    routine_param_m(len);

    huf_membuf_t *mem = (huf_membuf_t*)self->stream;
    *len = mem->len;

    routine_yield_m();
}


huf_error_t huf_memopen(huf_read_writer_t **self, void **buf, size_t capacity)
{
    routine_m();

    huf_error_t err;
    huf_membuf_t *mem;
    huf_read_writer_t *self_ptr;

    routine_param_m(self);
    routine_param_m(buf);

    // Allocate a buffer with the specified capacity.
    err = huf_malloc(void_pptr_m(buf), sizeof(void), capacity);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    err = huf_malloc(void_pptr_m(self), sizeof(huf_read_writer_t), 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    err = huf_malloc(void_pptr_m(&mem), sizeof(huf_membuf_t), 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    mem->buf = buf;
    mem->len = 0;
    mem->cap = capacity;

    self_ptr = *self;
    self_ptr->stream = mem;
    self_ptr->write = memwrite;
    self_ptr->read = memread;

    routine_yield_m();
}

huf_error_t huf_memclose(huf_read_writer_t **self)
{
    routine_m();
    routine_param_m(self);

    huf_read_writer_t *self_ptr = *self;
    huf_membuf_t *mem = (huf_membuf_t*)self_ptr->stream;

    free(mem);
    free(self_ptr);
    *self = NULL;

    routine_yield_m();
}
