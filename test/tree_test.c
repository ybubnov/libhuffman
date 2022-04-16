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

    printf("leaf %p\n", tree->leaves[3]);
    if (tree->leaves[3]) {
        printf("  value %d\n", tree->leaves[3]->index);
    }

    printf("root %p\n", tree->root);
    printf("  root (value) %d\n", tree->root->index);

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
