#include "huffman.h"

#include <unistd.h>
#include <string.h>
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

        shadow_tree[index1]->index = -index1;
        shadow_tree[index2]->index = index2;
        shadow_tree[node]->index = node;

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


int huf_serialize_tree(huf_node_t* tree, int16_t** dest) {
    if (tree) {
        **dest = tree->index;
        printf("%d ", **dest);

        (*dest)++;
        int l_len = huf_serialize_tree(tree->left, dest);

        (*dest)++;
        int r_len = huf_serialize_tree(tree->right, dest);

        return l_len + r_len + 1;
    } else {
        printf("0 ");

        **dest = 0;
        return 1;
    } 
}


int huf_deserialize_tree(huf_node_t* node, uint16_t* src) {
/*
 *    int16_t n_index = *src;
 *
 *    if (n_index != 0) {
 *        if (n_index > 0) {
 *            if ((node->right = (huf_node_t*)calloc(1, sizeof(huf_node_t*))) == 0) {
 *                return -1;
 *            }
 *
 *            if (huf_deserialize_tree(node->right, src++) != 0) {
 *                return -1;
 *            }
 *        } else {
 *            if ((node->left = (huf_node_t*)calloc(1, sizeof(huf_node_t))) == 0) {
 *                return -1;
 *            }
 *
 *            if (huf_deserialize_tree(node->left, src++) != 0) {
 *                return -1;
 *            }
 *        }
 *
 *        node->index = n_index;
 *    }
 */

    return 0;
}


int huf_create_table(huf_ctx_t* hctx, huf_decode_t** table)
{
    int index, position;
    char buf[65536];

    huf_decode_t* table_shadow;
    huf_node_t* pointer;

    *table = (huf_decode_t*)calloc(256, sizeof(huf_decode_t));
    table_shadow = *table;

    for (index = 0; index < 256; index++) {
        pointer = hctx->leaves[index];
        position = 0;

        while (pointer) {
            if (pointer->parent) {
                if (pointer->parent->left == pointer) {
                    buf[position] = '0'; 
                } else if (pointer->parent->right == pointer) {
                    buf[position] = '1';
                }

                position++;
            }

            pointer = pointer->parent;
        }

        if (position) {
            table_shadow[index].encoding = (char*)calloc(position + 1, sizeof(char));
            table_shadow[index].length = position;
            memcpy(table_shadow[index].encoding, buf, position);
            printf("%d: %s %d\n", index, table_shadow[index].encoding, position);
        }

    }

    return 0;
}

void huf_free_table(huf_decode_t* table)
{
    int index;

    for (index = 0; index < 256; index++) {
        free(table[index].encoding);
    }

    free(table);
}

void huf_write(huf_ctx_t* hctx, uint8_t* buf, uint64_t len, int flush)
{
    uint64_t i;

    huf_decode_t* table;
    huf_create_table(hctx, &table);
    huf_free_table(table);

    for (i = 0; i < len; i++) {
        printf("%c: ", buf[i]);

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
    int16_t* tree_shadow = (int16_t*)malloc(sizeof(uint16_t)*512);
    int16_t* tree_head = tree_shadow;

    huf_mktree(hctx);
    hctx->root->index = -1;
    int len = huf_serialize_tree(hctx->root, &tree_shadow);

    printf("\nLENGTH = %d\n", len);
    printf("\n");



    lseek(hctx->ifd, 0, SEEK_SET);

    do {
        if((obtained = read(hctx->ifd, buf, BUF_SIZE)) <= 0) {
            huf_write(hctx, buf, 0, 1);
            break;
        }

        huf_write(hctx, buf, obtained, 0);

        total += obtained;
    } while(total < hctx->length);

    free(tree_head);
    return 0;
}

