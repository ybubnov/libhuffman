#include "huffman/tree.h"
#include "huffman/malloc.h"
#include "huffman/sys.h"


huf_error_t
huf_tree_init(huf_tree_t **self)
{
    __try__;

    huf_tree_t *self_ptr;
    huf_error_t err;

    __argument__(self);

    err = huf_malloc((void**) self, sizeof(huf_tree_t), 1);
    __assert__(err);

    self_ptr = *self;

    err = huf_malloc((void**) self_ptr->leaves, sizeof(huf_node_t*), __HUFFMAN_ASCII_SYMBOLS * 2);
    __assert__(err);

    __finally__;
    __end__;
}


static void
__huf_tree_free(huf_node_t* node)
{
    if (!node) {
        return;
    }

    if (node->left) {
        __huf_tree_free(node->left);
        free(node->left);
    }

    if (node->right) {
        __huf_tree_free(node->right);
        free(node->right);
    }
}


huf_error_t
huf_tree_free(huf_tree_t **self)
{
    __try__;

    huf_tree_t *self_ptr;

    __argument__(self);

    self_ptr = *self;

    __huf_tree_free(self_ptr->root);

    free(self_ptr->leaves);
    free(self_ptr->root);
    free(self_ptr);

    *self = NULL;

    __finally__;
    __end__;
}


static huf_error_t
__huf_deserialize_tree(huf_node_t **node, int16_t **begin, const int16_t *end)
{
    __try__;

    huf_node_t **node_left;
    huf_node_t **node_right;
    huf_node_t *node_ptr;

    huf_error_t err;

    int16_t node_index;
    int16_t *begin_ptr = NULL;

    __argument__(node);
    __argument__(begin);
    __argument__(end);

    begin_ptr = *begin;

    if (begin_ptr + 1 > end) {
        __raise__(HUF_ERROR_INVALID_ARGUMENT);
    }

    node_ptr = *node;
    node_index = *begin_ptr;

    begin_ptr++;

    if (node_index != __HUFFMAN_LEAF) {
        err = huf_malloc((void**) &node_ptr, sizeof(huf_node_t), 1);
        __assert__(err);

        node_ptr->index = node_index;

        node_left = &(node_ptr->left);
        node_right = &(node_ptr->right);

        err = __huf_deserialize_tree(node_left, begin, end);
        __assert__(err);

        err = __huf_deserialize_tree(node_right, begin, end);
        __assert__(err);
    }

    __finally__;
    __end__;
}


huf_error_t
huf_tree_deserialize(huf_tree_t *self, int16_t *buf, size_t len)
{
    __try__;

    huf_error_t err;

    int16_t *buf_end;

    __argument__(self);
    __argument__(buf);

    buf_end = buf + len;

    err = __huf_deserialize_tree(&self->root, &buf, buf_end);
    __assert__(err);

    __finally__;
    __end__;
}


static huf_error_t
__huf_serialize_tree(const huf_node_t *node, int16_t *buf, size_t *len)
{
    __try__;

    huf_error_t err;
    int16_t *buf_ptr = NULL;

    size_t left_branch_len = 0;
    size_t right_branch_len = 0;

    __argument__(node);
    __argument__(buf);
    __argument__(len);

    buf_ptr = buf + 1;

    if (node) {
        *buf = node->index;

        err = __huf_serialize_tree(node->left, buf_ptr, &left_branch_len);
        __assert__(err);

        buf_ptr = buf + left_branch_len;

        err = __huf_serialize_tree(node->right, buf_ptr, &right_branch_len);
        __assert__(err);

    } else {
        *buf = __HUFFMAN_LEAF;
    }

    *len = left_branch_len + right_branch_len + 1;

    __finally__;
    __end__;
}


huf_error_t
huf_tree_serialize(huf_tree_t *self, int16_t *buf, size_t *len)
{
    __try__;

    huf_error_t err;

    __argument__(self);
    __argument__(buf);
    __argument__(len);

    err = __huf_serialize_tree(self->root, buf, len);
    __assert__(err);

    __finally__;
    __end__;
}
