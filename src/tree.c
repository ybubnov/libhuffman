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
    routine_m();

    const huf_node_t *node = self;
    size_t position = 0;

    routine_param_m(buf);
    routine_param_m(len);

    while(node) {
        if (!node->parent) {
            break;
        }

        // Keep borders of the buffer.
        if (position >= *len) {
            routine_success_m();
        }

        if (node->parent->left == node) {
            buf[position] = '0';
        } else {
            buf[position] = '1';
        }

        position++;
        node = node->parent;
    }

    routine_ensure_m();
    *len = position;

    routine_defer_m();
}


// Initialize a new instance of the Huffman tree.
huf_error_t
huf_tree_init(huf_tree_t **self)
{
    routine_m();
    routine_param_m(self);

    huf_error_t err = huf_malloc(void_pptr_m(self),
            sizeof(huf_tree_t), 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    huf_tree_t *self_ptr = *self;

    err = huf_malloc(void_pptr_m(&self_ptr->leaves),
            sizeof(huf_node_t*), HUF_ASCII_COUNT * 2);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    routine_yield_m();
}


// Recursively release memory occupied
// by the Huffman nodes.
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
    routine_m();
    routine_param_m(self);

    huf_tree_t *self_ptr = *self;

    __huf_tree_free(self_ptr->root);

    free(self_ptr->root);
    free(self_ptr->leaves);
    free(self_ptr);

    *self = NULL;

    routine_yield_m();
}


// Release the memory occupied by the Huffman
// tree nodes.
huf_error_t
huf_tree_reset(huf_tree_t *self)
{
    routine_m();
    routine_param_m(self);

    __huf_tree_free(self->root);
    free(self->root);
    self->root = NULL;

    // Reset the memory occupied by the leaves.
    memset(self->leaves, 0, (sizeof(huf_node_t*)
                * HUF_ASCII_COUNT * 2));

    routine_yield_m();
}


// Recursively de-serialize the Huffman tree into the
// provided buffer.
static huf_error_t
__huf_deserialize_tree(
        huf_node_t **node,
        const int16_t *buf, size_t *len)
{
    routine_m();

    int16_t node_index = HUF_LEAF_NODE;

    size_t left_branch_len = 0;
    size_t right_branch_len = 0;

    routine_param_m(node);
    routine_param_m(buf);
    routine_param_m(len);

    size_t buf_len = *len;

    if (buf_len < 1) {
        // Set length of the read data to zero, since there is
        // nothing to read;
        *len = 0;

        routine_success_m();
    }

    huf_node_t *node_ptr = *node;
    node_index = *buf;

    if (node_index == HUF_LEAF_NODE) {
        // Set length of the read data to one, since leaf is a
        // part of the serialized tree.
        *len = 1;

        routine_success_m();
    }

    huf_error_t err = huf_malloc(void_pptr_m(&node_ptr),
            sizeof(huf_node_t), 1);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    *node = node_ptr;
    node_ptr->index = node_index;

    huf_node_t **node_left = &node_ptr->left;
    huf_node_t **node_right = &node_ptr->right;

    const int16_t *buf_ptr = buf + 1;
    // Current node is de-serialized, so shift pointer to
    // the next one and reduce overall length of buffer by one.
    left_branch_len = buf_len - 1;

    // Recursively de-serialize a left branch of the tree.
    err = __huf_deserialize_tree(node_left,
            buf_ptr, &left_branch_len);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    buf_ptr += left_branch_len;
    right_branch_len = buf_len - left_branch_len - 1;

    // Recursively de-serialize a right branch of the tree.
    err = __huf_deserialize_tree(node_right,
            buf_ptr, &right_branch_len);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    // Return in *len* argument length of the read data.
    *len = left_branch_len + right_branch_len + 1;

    routine_yield_m();
}


// De-serialize the Huffman tree into the
// provided buffer.
huf_error_t
huf_tree_deserialize(
        huf_tree_t *self,
        const int16_t *buf, size_t len)
{
    routine_m();

    routine_param_m(self);
    routine_param_m(buf);

    huf_error_t err = __huf_deserialize_tree(
            &self->root, buf, &len);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    routine_yield_m();
}



// Recursively serialize the Huffman tree from the
// provided buffer.
static huf_error_t
__huf_serialize_tree(
        const huf_node_t *node,
        int16_t *buf, size_t *len)
{
    routine_m();

    size_t left_branch_len = 0;
    size_t right_branch_len = 0;

    routine_param_m(buf);
    routine_param_m(len);

    if (node) {
        *buf = node->index;

        int16_t *buf_ptr = buf + 1;

        // Serialize the left branch of the tree.
        huf_error_t err = __huf_serialize_tree(
                node->left, buf_ptr, &left_branch_len);
        if (err != HUF_ERROR_SUCCESS) {
            routine_error_m(err);
        }

        buf_ptr += left_branch_len;

        // Serialize the right branch of the tree.
        err = __huf_serialize_tree(node->right,
                buf_ptr, &right_branch_len);
        if (err != HUF_ERROR_SUCCESS) {
            routine_error_m(err);
        }

    } else {
        *buf = HUF_LEAF_NODE;
    }

    *len = left_branch_len + right_branch_len + 1;

    routine_yield_m();
}


// Serialize the Huffman tree from the
// provided buffer.
huf_error_t
huf_tree_serialize(
        huf_tree_t *self,
        int16_t *buf, size_t *len)
{
    routine_m();

    routine_param_m(self);
    routine_param_m(buf);
    routine_param_m(len);

    huf_error_t err = __huf_serialize_tree(
            self->root, buf, len);
    if (err != HUF_ERROR_SUCCESS) {
        routine_error_m(err);
    }

    routine_yield_m();
}
