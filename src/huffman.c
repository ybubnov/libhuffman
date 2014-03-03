#include "huffman.h"

#include <unistd.h>
#include <stdio.h>


int huf_mktree(huf_ctx_t* hctx) 
{    
    int i, j, index, start = 256;
    uint8_t buf[BUF_SIZE];
    int64_t* rates;
    uint64_t total = 0, obtained = 0;

    if ((rates = (int64_t*)calloc(512, sizeof(int64_t))) == 0) {
        return -1;
    }

    do {
        if((obtained = read(hctx->fd, buf, BUF_SIZE)) <= 0) {
            break;
        }

        for (i = 0; i < obtained; i++) {
            index = buf[i];
            rates[index]++;

            if (index < start) {
                start = index;
            }
        }

        total += obtained;
    } while(total < hctx->length);


    int64_t rate, rate1, rate2;
    int16_t index1, index2, node = 256;
    huf_node_t** shadow_tree;

   if ((shadow_tree = (huf_node_t**)calloc(512, sizeof(huf_node_t*))) == 0) {
       return -1;
   } 

    for (i = start; i < 512; i++) {
        index1 = index2 = -1;
        rate1 = rate2 = 0;

        for (j = i; j < node; j++) {
            rate = rates[j];

            if (rate) {
                if (!rate1) {
                    rate1 = rate;
                    index1 = j;
                } else if (!rate2) {
                    rate2 = rate;
                    index2 = j;
                } else if (rate <= rate1) {
                    rate2 = rate1;
                    rate1 = rate;
                    index2 = index1;
                    index1 = j;
                } else if (rate <= rate2) {
                    rate2 = rate;
                    index2 = j;
                }
            }
        }


        if (index1 == -1 || index2 == -1) {
            hctx->root = shadow_tree[node - 1];
            break;
        }

        if (!shadow_tree[index1]) {
            if ((shadow_tree[index1] = (huf_node_t*)calloc(1, sizeof(huf_node_t))) == 0) {
                return -1;
            }
        }

        if (!shadow_tree[index2]) {
            if ((shadow_tree[index2] = (huf_node_t*)calloc(1, sizeof(huf_node_t))) == 0) {
                return -1;
            }
        }

        if ((shadow_tree[node] = (huf_node_t*)calloc(1, sizeof(huf_node_t))) == 0) {
            return -1;
        }

        if (index1 < 256) {
            hctx->leaves[index1] = shadow_tree[index1];
        }

        if (index2 < 256) {
            hctx->leaves[index2] = shadow_tree[index1];
        }

        shadow_tree[index1]->parent = shadow_tree[node];
        shadow_tree[index2]->parent = shadow_tree[node];
        shadow_tree[node]->left = shadow_tree[index1];
        shadow_tree[node]->right = shadow_tree[index2];

        rates[node] = rate1 + rate2;
        rates[index1] = 0;
        rates[index2] = 0;
        node++;
    }


    free(shadow_tree);
    free(rates);
    return 0;
}

int huf_init(int fd, uint64_t length, huf_ctx_t* hctx) 
{
    hctx->fd = fd;
    hctx->length = length;
    hctx->root = 0;
    
    if ((hctx->leaves = (huf_node_t**)calloc(256, sizeof(huf_node_t*))) == 0) {
        return -1;
    }

    return 0;
}

void huf_free_tree(huf_node_t* node)
{
    if (node->left) {
        huf_free_tree(node->left);
        free(node->left);
    }
    
    if (node->right) {
        huf_free_tree(node->right);
        free(node->right);
    }
}

void huf_free(huf_ctx_t* hctx)
{ 
    huf_free_tree(hctx->root);
    free(hctx->root);
    free(hctx->leaves);
}


int huf_decode(huf_ctx_t* hctx)
{
    huf_mktree(hctx);
    return 0;
}

