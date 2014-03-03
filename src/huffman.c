#include "huffman.h"

#include <unistd.h>
#include <stdio.h>

uint8_t huf_write_buffer[BUF_SIZE];
int huf_buffer_pos = 0;


int huf_mktree(huf_ctx_t* hctx) 
{    
    int j, index, start = 256;
    uint8_t buf[BUF_SIZE];
    int64_t* rates;
    uint64_t i, total = 0, obtained = 0;

    if ((rates = (int64_t*)calloc(512, sizeof(int64_t))) == 0) {
        return -1;
    }

    do {
        if((obtained = read(hctx->ifd, buf, BUF_SIZE)) <= 0) {
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

        /*
         *printf("rate1:  %5lld\t%5d\trate2:  %5lld\t%5d\n", 
         *        (long long)rate1, index1, (long long)rate2, index2);
         */

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
            hctx->leaves[index2] = shadow_tree[index2];
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


int huf_init(int ifd, int ofd, uint64_t length, huf_ctx_t* hctx) 
{
    hctx->ifd = ifd;
    hctx->ofd = ofd;
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


uint8_t* huf_serialize_tree(huf_node_t* tree) {
    return 0;
}


huf_node_t* huf_deserialize_tree(uint8_t* buf) {
    return 0;
}


void huf_write(huf_ctx_t* hctx, uint8_t* buf, uint64_t len, int flush)
{
    uint64_t i;
    huf_node_t* pointer;


    for (i = 0; i < len; i++) {
        printf("%c: ", buf[i]);

        pointer = hctx->leaves[buf[i]];
        while(pointer) {

            if (pointer->parent) {
                if (pointer->parent->left == pointer) {
                    printf("0");
                }

                if (pointer->parent->right == pointer) {
                    printf("1");
                }
            }

            pointer = pointer->parent;
        }

        printf("\n");


        // ...
    }

    if (flush) {
        // flush to disk
    }
}


int huf_decode(huf_ctx_t* hctx)
{
    uint8_t buf[BUF_SIZE];
    uint64_t obtained, total = 0;

    // write serialized tree to file

    huf_mktree(hctx);

    huf_serialize_tree(*(hctx->leaves));
    lseek(hctx->ifd, 0, SEEK_SET);

    do {
        if((obtained = read(hctx->ifd, buf, BUF_SIZE)) <= 0) {
            huf_write(hctx, buf, 0, 1);
            break;
        }

        huf_write(hctx, buf, obtained, 0);

        total += obtained;
    } while(total < hctx->length);

    return 0;
}

