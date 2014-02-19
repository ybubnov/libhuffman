#include "huffman.h"

#include <unistd.h>
#include <stdio.h>

int huf_mktree(huf_ctx_t hctx) 
{    
    int i, j, index, start = 256;
    uint8_t buf[BUF_SIZE];
    int64_t* rates;
    uint64_t total = 0, obtained = 0;

    if ((rates = (int64_t*)calloc(512, sizeof(int64_t))) == 0) {
        return -1;
    }

    do {
        if((obtained = read(hctx.fd, buf, BUF_SIZE)) <= 0) {
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
    } while(total < hctx.length);


    int64_t rate, rate1, rate2;
    int16_t parent, index1, index2, node = 256;

    for (i = start; i < 512; i++) {
        index1 = index2 = -1;
        rate1 = rate2 = 0;

        for (j = i; j < node; j++) {
            parent = hctx.tree[j].parent;
            rate = rates[j];

            if (rate && parent == -1) {
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

        printf("rate1:  %5lld\t%5d\trate2:  %5lld\t%5d\n", 
                (long long)rate1, index1, (long long)rate2, index2);

        if (index1 == -1 || index2 == -1) {
            break;
        }

        hctx.tree[index1].parent = node;
        hctx.tree[index2].parent = node;
        hctx.tree[node].left = index1;
        hctx.tree[node].right = index2;

        rates[node] = rate1 + rate2;
        node++;
    }


    for (i = 0; i < 512; i++) {
        printf("%5d:\t%8lld\t%6d\n", i, (long long)rates[i], hctx.tree[i].parent);
    } 

    free(rates);
    return 0;
}

int huf_init(int fd, uint64_t length, huf_ctx_t* hctx) 
{
    int i;
    hctx->fd = fd;
    hctx->length = length;
    hctx->msize = 0;
    
    if ((hctx->tree = (huf_node*)calloc(512, sizeof(huf_node))) == 0) {
        return -1;
    }

    for (i = 0; i < 512; i++) {
        hctx->tree[i].parent = -1;
        hctx->tree[i].left = -1;
        hctx->tree[i].right = -1;
    }

    return 0;
}

void huf_free(huf_ctx_t* hctx)
{ 
    free(hctx->tree);
}


int huf_decode(huf_ctx_t hctx)
{
    huf_mktree(hctx);
    return 0;
}
