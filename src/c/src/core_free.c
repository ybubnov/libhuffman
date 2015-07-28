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
huf_free(huf_ctx_t* hctx)
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
        __huf_free_tree(hctx->root);
    }

    if (hctx->root) {
        free(hctx->root);
    }

    free(hctx->leaves);
}
