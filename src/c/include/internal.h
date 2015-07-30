#ifndef INTERNAL_H
#define INTERNAL_H


typedef struct __huf_args {
    int ifd;
    int ofd;
    uint64_t len;
} huf_args_t;


huf_error_t
huf_write(int fd, const char *buf, size_t count);

huf_error_t
huf_read(int fd, char *buf, size_t count);

huf_error_t
huf_alloc(void* ptr, size_t size, size_t num);


#endif // INTERNAL_H
