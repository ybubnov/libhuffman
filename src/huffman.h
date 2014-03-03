#ifndef HUFFMAN_H
#define HUFFMAN_H

#define _LARGEFILE64_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdint.h>


#define BUF_SIZE 65536

typedef struct huf_node_t {
    struct huf_node_t* parent;
    struct huf_node_t* left;
    struct huf_node_t* right;
} huf_node_t;


typedef struct {
    int fd;
    uint64_t length;
    huf_node_t** leaves;
    huf_node_t* root;
} huf_ctx_t;

int huf_init(int fd, uint64_t length, huf_ctx_t* hctx);
int huf_decode(huf_ctx_t* hctx);
int huf_encode(huf_ctx_t hctx);
void huf_free(huf_ctx_t* hctx);


#endif //HUFFMAN_H
