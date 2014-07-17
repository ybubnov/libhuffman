#include "huffman.h"
#include "error.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define LEAF 1024

uint8_t huf_write_buffer[BUF_SIZE];
uint32_t huf_write_pos = 0;
uint8_t huf_bit_buffer = 0;
uint8_t huf_bit_pos = 7;
huf_node_t* huf_last_node = 0;


int huf_mktree(huf_ctx_t* hctx)
{
    int j, index, start = 256;
    uint8_t buf[BUF_SIZE];
    int64_t* rates;
    uint64_t i, total = 0, obtained = 0;

    if ((rates = (int64_t*)calloc(512, sizeof(int64_t))) == 0) {
        ERROR("Memory allocation failed. %s %d\n", __FILE__, __LINE__);
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
        ERROR("Memory allocation failed. %s %d\n", __FILE__, __LINE__);
        return -1;
    }


    while (start < 512) {
        index1 = index2 = -1;
        rate1 = rate2 = 0;

        while(!rates[start]) {
            start++;
        }

        for (j = start; j < node; j++) {
            rate = rates[j];

            if (rate) {
                if (!rate1) {
                    rate1 = rate;
                    index1 = j;
                } else if (rate <= rate1) {
                    rate2 = rate1;
                    rate1 = rate;
                    index2 = index1;
                    index1 = j;
                } else if (!rate2 || rate <= rate2) {
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
                ERROR("Memory allocation failed. %s %d\n", __FILE__, __LINE__);
                return -1;
            }
        }

        if (!shadow_tree[index2]) {
            if ((shadow_tree[index2] = (huf_node_t*)calloc(1, sizeof(huf_node_t))) == 0) {
                ERROR("Memory allocation failed. %s %d\n", __FILE__, __LINE__);
                return -1;
            }
        }

        if ((shadow_tree[node] = (huf_node_t*)calloc(1, sizeof(huf_node_t))) == 0) {
            ERROR("Memory allocation failed. %s %d\n", __FILE__, __LINE__);
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

        /*shadow_tree[index1]->index = -index1;*/
        /*shadow_tree[index2]->index = index2;*/
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
    hctx->table = 0;

    if ((hctx->leaves = (huf_node_t**)calloc(256, sizeof(huf_node_t*))) == 0) {
        ERROR("Memory allocation failed. %s %d\n", __FILE__, __LINE__);
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
    int index;

    if (hctx->table) {
        for (index = 0; index < 256; index++) {
            free(hctx->table[index].encoding);
        }
    }

    if (hctx->table) {
        free(hctx->table);
    }

    if (hctx->root) {
        huf_free_tree(hctx->root);
    }

    if (hctx->root) {
        free(hctx->root);
    }

    free(hctx->leaves);
}


int16_t huf_serialize_tree(huf_node_t* tree, int16_t** dest)
{
    if (tree) {
        **dest = tree->index;

        (*dest)++;
        int l_len = huf_serialize_tree(tree->left, dest);

        (*dest)++;
        int r_len = huf_serialize_tree(tree->right, dest);

        return l_len + r_len + 1;
    } else {
        **dest = LEAF;
        return 1;
    }
}


int huf_deserialize_tree(huf_node_t** node, int16_t** src, int16_t* src_end)
{
    int16_t n_index = **src;
    huf_node_t** node_left;
    huf_node_t** node_right;

    if ((*src) + 1 > src_end) {
        ERROR("Unexpected end of buffer.\n");
        return -1;
    }

    (*src)++;

    if (n_index != LEAF) {
        if (((*node) = (huf_node_t*)calloc(1, sizeof(huf_node_t))) == 0) {
            ERROR("Memory allocation failed. %s %d\n", __FILE__, __LINE__);
            return -1;
        }

        (*node)->index = n_index;
        node_left = &((*node)->left);
        node_right = &((*node)->right);

        if (huf_deserialize_tree(node_left, src, src_end) != 0) {
            return -1;
        }

        if (huf_deserialize_tree(node_right, src, src_end) != 0) {
            return -1;
        }
    }

    return 0;
}


int huf_create_table(huf_ctx_t* hctx)
{
    int index, position;
    char buf[65536];
    huf_node_t* pointer;

    if ((hctx->table = (huf_encode_t*)calloc(256, sizeof(huf_encode_t))) == 0) {
        ERROR("Memory allocation failed.\n");
        return -1;
    }

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
            if ((hctx->table[index].encoding = (char*)calloc(position + 1, sizeof(char))) == 0) {
                ERROR("Memory allocation failed.\n");
                return -1;
            }

            hctx->table[index].length = position;
            memcpy(hctx->table[index].encoding, buf, position);
        }

    }

    return 0;
}


int huf_encode_partial(huf_ctx_t* hctx, uint8_t* buf, uint64_t len)
{
    uint64_t pos;
    int length, index;
    char* encoding;

    if (!hctx->table) {
        ERROR("Unexpected error.\nInvalid encoding table.\n");
        return -1;
    }

    for (pos = 0; pos < len; pos++) {
        encoding = hctx->table[buf[pos]].encoding;
        length = hctx->table[buf[pos]].length;

        for (index = length; index > 0; index--) {
            huf_bit_buffer |= ((encoding[index - 1] & 1) << huf_bit_pos);

            if (!huf_bit_pos) {
                if (huf_write_pos >= BUF_SIZE) {
                    if (write(hctx->ofd, huf_write_buffer, BUF_SIZE) == -1) {
                        ERROR("Failed writing buffer to file.\n");
                        return -1;
                    }

                    huf_write_pos = 0;
                }

                huf_write_buffer[huf_write_pos] = huf_bit_buffer;
                huf_write_pos++;
                huf_bit_buffer = 0;
                huf_bit_pos = 8;
            }

            huf_bit_pos--;
        }
    }

    return 0;
}


int huf_encode_flush(huf_ctx_t* hctx)
{
    if (huf_bit_pos != 7) {
        huf_write_buffer[huf_write_pos++] = huf_bit_buffer;
    }

    return write(hctx->ofd, huf_write_buffer, huf_write_pos);
}


int huf_encode(huf_ctx_t* hctx)
{
    uint8_t buf[BUF_SIZE], total = 0;
    int64_t obtained;

    int16_t* tree_shadow = (int16_t*)malloc(sizeof(uint16_t)*1024);
    int16_t* tree_head = tree_shadow;

    if (huf_mktree(hctx) != 0) {
        ERROR("Failed to create huffman tree.\n");
        return -1;
    }

    if (huf_create_table(hctx) != 0) {
        ERROR("Failed to create encoding table.\n");
        return -1;
    }

    /*hctx->root->index = -1024;*/
    int16_t len = huf_serialize_tree(hctx->root, &tree_shadow);

    huf_write_pos = sizeof(hctx->length) + sizeof(len) + len * sizeof(*tree_head);
    memcpy(huf_write_buffer, &hctx->length, sizeof(hctx->length));
    memcpy(huf_write_buffer + sizeof(hctx->length), &len, sizeof(len));
    memcpy(huf_write_buffer + sizeof(hctx->length) + sizeof(len),
            tree_head, len * sizeof(*tree_head));

    lseek(hctx->ifd, 0, SEEK_SET);

    do {
        if((obtained = read(hctx->ifd, buf, BUF_SIZE)) <= 0) {
            break;
        }

        if (huf_encode_partial(hctx, buf, obtained) != 0) {
            ERROR("Encoding failed.\n");
            return -1;
        }

        total += obtained;
    } while(total < hctx->length);

    if (huf_encode_flush(hctx) == -1) {
        ERROR("Failed writing buffer to a file.\n");
        return -1;
    }

    free(tree_head);
    return 0;
}


int huf_decode_partial(const huf_ctx_t* hctx, uint8_t* buf, uint64_t len, uint64_t* written)
{
    uint64_t pos;
    uint8_t bit_buffer;
    uint8_t bit_position;

    *written = 0;

    if (huf_last_node == 0) {
        huf_last_node = hctx->root;
    }

    for (pos = 0; pos < len; pos++) {
        bit_buffer = buf[pos];

        for (bit_position = 8; bit_position > 0; bit_position--) {
            if ((bit_buffer >> (bit_position - 1)) & 1) {
                huf_last_node = huf_last_node->right;
            } else {
                huf_last_node = huf_last_node->left;
            }

            if (!huf_last_node->left && !huf_last_node->right) {
                if (huf_write_pos >= BUF_SIZE) {
                    if (write(hctx->ofd, huf_write_buffer, BUF_SIZE) == -1) {
                        ERROR("Failed writing buffer to a file.\n");
                        return -1;
                    }

                    *written += BUF_SIZE;
                    huf_write_pos = 0;
                }

                //put letter into buffer
                huf_write_buffer[huf_write_pos++] = huf_last_node->index;
                huf_last_node = hctx->root;
            }
        }
    }

    return 0;
}


int huf_decode_flush(huf_ctx_t* hctx, int extra)
{
    return write(hctx->ofd, huf_write_buffer, extra);
}


int huf_decode(huf_ctx_t* hctx)
{
    uint8_t buf[BUF_SIZE];
    uint64_t obtained, total = 0;
    uint64_t if_length, written;

    int16_t tree_length = 0;
    int16_t *tree_head, *tree_shadow;
    huf_node_t* root;

    if ((read(hctx->ifd, &if_length, sizeof(hctx->length))) == -1) {
        ERROR("Failed to read source file length.\nBroken file?\n");
        return -1;
    }

    if (read(hctx->ifd, &tree_length, sizeof(tree_length)) == -1) {
        ERROR("Failed to read tree length.\nBroken file?\n");
        return -1;
    }

    if ((tree_head = calloc(tree_length, sizeof(*tree_head))) == 0) {
        ERROR("Memory allocation failed. %s %d\n", __FILE__, __LINE__);
        return -1;
    }

    if (read(hctx->ifd, tree_head, tree_length * sizeof(*tree_head)) == -1) {
        ERROR("Failed to read tree.\nBroken file?\n");
        return -1;
    }

    tree_shadow = tree_head;
    if (huf_deserialize_tree(&root, &tree_shadow, tree_head + tree_length) != 0) {
        ERROR("Tree deserialization failed.\n");
        return -1;
    }

    hctx->root = root;
    huf_write_pos = 0;
    huf_last_node = 0;

    do {
        if((obtained = read(hctx->ifd, buf, BUF_SIZE)) <= 0) {
            break;
        }

        if (huf_decode_partial(hctx, buf, obtained, &written) != 0) {
            ERROR("Decoding failed.\n");
            return -1;
        }

        if_length -= written;
        total += obtained;
    } while(total < hctx->length);

    if (huf_decode_flush(hctx, if_length) == -1) {
        ERROR("Failed writing buffer to a file.\n");
        return -1;
    }

    free(tree_head);

    return 0;
}

