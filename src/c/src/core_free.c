#include <stdlib.h>

#include "core.h"


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


void
huf_free(huf_ctx_t* self)
{
    int index;

    if (self->table) {
        for (index = 0; index < 256; index++) {
            free(self->table[index].encoding);
        }
    }

    if (self->table) {
        free(self->table);
    }

    if (self->root) {
        __huf_free_tree(self->root);
    }

    if (self->root) {
        free(self->root);
    }

    free(self->leaves);
}
