#ifndef HUFFMAN_H
#define HUFFMAN_H

#define _LARGEFILE64_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdint.h>

#define UINT32_LIMIT 0xffffffff
#define UINT16_LIMIT 0xffff
#define UINT8_LIMIT 0xff

#define BUF_SIZE 65536

typedef struct {
    int16_t parent;
    int16_t left;
    int16_t right;
} huf_node;

typedef struct {
    int fd;
    uint64_t length;
    huf_node* tree;
    uint64_t msize;
} huf_ctx_t;

int huf_init(int fd, uint64_t length, huf_ctx_t* hctx);
int huf_decode(huf_ctx_t hctx);
int huf_encode(huf_ctx_t hctx);
void huf_free(huf_ctx_t* hctx);


#endif //HUFFMAN_H
