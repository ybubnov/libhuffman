#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>

#include <huffman/histogram.h>
#include <huffman/tree.h>
#include "assert.h"
#include <stdio.h>

static void
test_tree_from_histogram(void **state)
{
    huf_histogram_t *hist = NULL;
    huf_tree_t *tree = NULL;

    assert_ok(huf_histogram_init(&hist, 4, 10));
    assert_ok(huf_tree_init(&tree));

    uint32_t array[] = {3, 3, 3, 3};
    assert_ok(huf_histogram_populate(hist, array, sizeof(array)));
    assert_ok(huf_tree_from_histogram(tree, hist));

    assert_non_null(tree->leaves[3]);
    assert_int_equal(tree->leaves[3]->index, 3);

    assert_non_null(tree->root);
    assert_int_equal(tree->root->index, 256);
    assert_ptr_equal(tree->root->left, tree->leaves[3]);
    assert_null(tree->root->right);

    assert_ok(huf_histogram_free(&hist));
    assert_ok(huf_tree_free(&tree));
}


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_tree_from_histogram),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
