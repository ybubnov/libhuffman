#include <stdlib.h>

#include <huffman/huffman.h>
#include <huffman/runtime/sys.h>


static void
__huf_free_tree(huf_node_t* node)
{
    if (node->left) {
        __huf_free_tree(node->left);
        free(node->left);
    }

    if (node->right) {
        __huf_free_tree(node->right);
        free(node->right);
    }
}


huf_error_t
huf_free(huf_archiver_t **self)
{
    __try__;

    huf_archiver_t *self_pointer;
    int index;

    __argument__(self);

    self_pointer = *self;

    if (self_pointer->root) {
        __huf_free_tree(self_pointer->root);
    }

    if (self_pointer->char_coding) {
        for (index = 0; index < 256; index++) {
            free(self_pointer->char_coding[index].encoding);
        }
    }

    free(self_pointer->char_coding);
    free(self_pointer->root);
    free(self_pointer->leaves);

    *self = 0;

    __finally__;
    __end__;
}
