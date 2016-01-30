#include <string.h>

#include "huffman/tree.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


// Write the path to the root node of the Huffman
// tree starting from the specified node into the
// buffer.
huf_error_t
huf_node_to_string(
        const huf_node_t *self,
        uint8_t *buf, size_t *len)
{
    __try__;

    const huf_node_t *node = self;
    size_t position = 0;

    __argument__(buf);
    __argument__(len);

    while(node) {
        if (!node->parent) {
            break;
        }

        // Keep borders of the buffer.
        if (position >= *len) {
            __success__;
        }

        if (node->parent->left == node) {
            buf[position] = '0';
        } else {
            buf[position] = '1';
        }

        position++;
        node = node->parent;
    }

    __finally__;

    *len = position;

    __end__;
}


// Initialize a new instance of the Huffman tree.
huf_error_t
huf_tree_init(huf_tree_t **self)
{
    __try__;

    huf_tree_t *self_ptr;
    huf_error_t err;

    __argument__(self);

    err = huf_malloc(void_pptr_m(self),
            sizeof(huf_tree_t), 1);
    __assert__(err);

    self_ptr = *self;

    err = huf_malloc(void_pptr_m(&self_ptr->leaves),
            sizeof(huf_node_t*), __HUFFMAN_ASCII_SYMBOLS * 2);
    __assert__(err);

    __finally__;
    __end__;
}


// Release memory occupied by the Huffman nodes.
static void
__huf_tree_free(huf_node_t* node)
{
    if (!node) {
        return;
    }

    if (node->left) {
        __huf_tree_free(node->left);
        free(node->left);
        node->left = NULL;
    }

    if (node->right) {
        __huf_tree_free(node->right);
        free(node->right);
        node->left = NULL;
    }
}


// Release memory occupied by the Huffman tree.
huf_error_t
huf_tree_free(huf_tree_t **self)
{
    __try__;

    huf_tree_t *self_ptr;

    __argument__(self);

    self_ptr = *self;

    __huf_tree_free(self_ptr->root);

    free(self_ptr->root);
    free(self_ptr->leaves);
    free(self_ptr);

    *self = NULL;

    __finally__;
    __end__;
}


// Release the memory occupied by the Huffman
// tree nodes.
huf_error_t
huf_tree_reset(huf_tree_t *self)
{
    __try__;

    __argument__(self);

    __huf_tree_free(self->root);

    self->root = NULL;

    __finally__;
    __end__;
}


// Recursively de-serialize the Huffman tree into the
// provided buffer.
static huf_error_t
__huf_deserialize_tree(
        huf_node_t **node,
        const int16_t *buf, size_t *len)
{
    __try__;

    huf_node_t **node_left = NULL;
    huf_node_t **node_right = NULL;
    huf_node_t *node_ptr = NULL;

    huf_error_t err;

    const int16_t *buf_ptr = NULL;
    size_t buf_len = 0;

    int16_t node_index = __HUFFMAN_LEAF;

    size_t left_branch_len = 0;
    size_t right_branch_len = 0;

    __argument__(node);
    __argument__(buf);
    __argument__(len);

    buf_len = *len;

    if (buf_len < 1) {
        // Set length of the read data to zero, since there is
        // nothing to read;
        *len = 0;

        __raise__(HUF_ERROR_SUCCESS);
    }

    node_ptr = *node;
    node_index = *buf;

    if (node_index == __HUFFMAN_LEAF) {
        // Set length of the read data to one, since leaf is a
        // part of the serialized tree.
        *len = 1;

        __raise__(HUF_ERROR_SUCCESS);
    }

    err = huf_malloc(void_pptr_m(&node_ptr),
            sizeof(huf_node_t), 1);
    __assert__(err);

    *node = node_ptr;

    node_ptr->index = node_index;

    node_left = &node_ptr->left;
    node_right = &node_ptr->right;

    buf_ptr = buf + 1;
    // Current node is de-serialized, so shift pointer to
    // the next one and reduce overall length of buffer by one.
    left_branch_len = buf_len - 1;

    // Recursively de-serialize a left branch of the tree.
    err = __huf_deserialize_tree(node_left,
            buf_ptr, &left_branch_len);
    __assert__(err);

    buf_ptr += left_branch_len;
    right_branch_len = buf_len - left_branch_len - 1;

    // Recursively de-serialize a right branch of the tree.
    err = __huf_deserialize_tree(node_right,
            buf_ptr, &right_branch_len);
    __assert__(err);

    // Return in *len* argument length of the read data.
    *len = left_branch_len + right_branch_len + 1;

    __finally__;
    __end__;
}


// De-serialize the Huffman tree into the
// provided buffer.
huf_error_t
huf_tree_deserialize(
        huf_tree_t *self,
        const int16_t *buf, size_t len)
{
    __try__;

    huf_error_t err;

    __argument__(self);
    __argument__(buf);

    err = __huf_deserialize_tree(&self->root,
            buf, &len);
    __assert__(err);

    __finally__;
    __end__;
}



// Recursively serialize the Huffman tree from the
// provided buffer.
static huf_error_t
__huf_serialize_tree(
        const huf_node_t *node,
        int16_t *buf, size_t *len)
{
    __try__;

    huf_error_t err;
    int16_t *buf_ptr = NULL;

    size_t left_branch_len = 0;
    size_t right_branch_len = 0;

    __argument__(buf);
    __argument__(len);

    if (node) {
        *buf = node->index;

        buf_ptr = buf + 1;

        // Serialize the left branch of the tree.
        err = __huf_serialize_tree(node->left,
                buf_ptr, &left_branch_len);
        __assert__(err);

        buf_ptr += left_branch_len;

        // Serialize the right branch of the tree.
        err = __huf_serialize_tree(node->right,
                buf_ptr, &right_branch_len);
        __assert__(err);

    } else {
        *buf = __HUFFMAN_LEAF;
    }

    *len = left_branch_len + right_branch_len + 1;

    __finally__;
    __end__;
}


// Serialize the Huffman tree from the
// provided buffer.
huf_error_t
huf_tree_serialize(
        huf_tree_t *self,
        int16_t *buf, size_t *len)
{
    __try__;

    huf_error_t err;

    __argument__(self);
    __argument__(buf);
    __argument__(len);

    err = __huf_serialize_tree(self->root,
            buf, len);
    __assert__(err);

    __finally__;
    __end__;
}