#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <huffman/histogram.h>

static void test_histogram_allocation(void **state)
{
    huf_histogram_t *histogram = NULL;

    // Validate the memory allocation for the histogram structure.
    huf_histogram_init(&histogram, 2, 10);
    assert_non_null(histogram);
    assert_non_null(histogram->frequencies);

    // Validate the initial values of the structure members.
    assert_true(histogram->iota == 2);
    assert_true(histogram->length == 10);
    assert_true(histogram->start == -1);

    // Release resource occupied by the histogram.
    huf_histogram_free(&histogram);
    assert_null(histogram);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_histogram_allocation),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
