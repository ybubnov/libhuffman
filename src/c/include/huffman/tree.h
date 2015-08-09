#ifndef INCLUDE_huffman_tree_h__
#define INCLUDE_huffman_tree_h__

#include "huffman/common.h"
#include "huffman/errors.h"

/* Default buffer size for write operations.
 */
#define __HUFFMAN_DEFAULT_BUFFER 65536
#define __HUFFMAN_ASCII_SYMBOLS 256

/* All leafs of the huffman tree will be marked with that value.
 */
#define __HUFFMAN_LEAF 1024


typedef struct __huf_node {
    int16_t index;
    struct __huf_node* parent;
    struct __huf_node* left;
    struct __huf_node* right;
} huf_node_t;


typedef struct __huf_char_coding {
    int length;
    uint8_t* encoding;
} huf_char_coding_t;


typedef struct __huf_tree {
    /* List of Huffmat tree leaves.
     */
    huf_node_t **leaves;

    /* root element of the Huffman tree.
     */
    huf_node_t *root;
} huf_tree_t;


huf_error_t
huf_tree_init(huf_tree_t **self);


huf_error_t
huf_tree_free(huf_tree_t **self);


huf_error_t
huf_tree_deserialize(huf_tree_t *self, int16_t *buf, size_t len);


huf_error_t
huf_tree_serialize(huf_tree_t *self, int16_t *buf, size_t *len);


#endif // INCLUDE_huffman_tree_h__
