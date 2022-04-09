#ifndef INCLUDE_huffman_tree_h__
#define INCLUDE_huffman_tree_h__

#include "huffman/common.h"
#include "huffman/errors.h"
#include "huffman/histogram.h"

// The count of ASCII symbols
#define HUF_ASCII_COUNT 256

// Maximum length of the 2-byte serialized Huffman tree.
#define HUF_BTREE_LEN 1024

// Length of the symbols frequency histogram.
#define HUF_HISTOGRAM_LEN 512

// All leaves of the Huffman tree will be marked with that value.
#define HUF_LEAF_NODE -1


#define CFFI_huffman_tree_h__

// A node of the Huffman tree.
typedef struct __huf_node {
    // The weight of the node.
    int16_t index;

    // The pointer to the parent node.
    struct __huf_node *parent;

    // The pointer to the left child node.
    struct __huf_node *left;

    // The pointer to the right child node.
    struct __huf_node *right;
} huf_node_t;


// Write the path to the root node of the Huffman
// tree starting from the specified node into the
// buffer.
huf_error_t
huf_node_to_string(const huf_node_t *self, uint8_t *buf, size_t *len);


// A Huffman tree.
typedef struct __huf_tree {
    // List of Huffman tree leaves.
    huf_node_t **leaves;

    // Root element of the Huffman tree.
    huf_node_t *root;
} huf_tree_t;


// Initialize a new instance of the Huffman tree.
huf_error_t
huf_tree_init(huf_tree_t **self);


// Release memory occupied by the Huffman tree.
huf_error_t
huf_tree_free(huf_tree_t **self);


// Release the memory occupied by the Huffman tree nodes.
huf_error_t
huf_tree_reset(huf_tree_t *self);


// De-serialize the Huffman tree into the provided buffer.
huf_error_t
huf_tree_deserialize(huf_tree_t *self, const int16_t *buf, size_t len);


// Serialize the Huffman tree from the provided buffer.
huf_error_t
huf_tree_serialize(huf_tree_t *self, int16_t *buf, size_t *len);


huf_error_t
huf_tree_from_histogram(huf_tree_t *self, huf_histogram_t *histogram);


#undef CFFI_huffman_tree_h__
#endif // INCLUDE_huffman_tree_h__
