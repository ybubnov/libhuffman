#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <huffman/histogram.h>


// Validate the memory allocation for the histogram structure.
static void
test_histogram_allocation(void **state)
{
    huf_histogram_t *histogram = NULL;

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


// Validate the histogram data population.
static void
test_histogram_populate(void **state)
{
    huf_histogram_t *histogram = NULL;

    huf_histogram_init(&histogram, 4, 10);
    assert_non_null(histogram);

    uint32_t array1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    huf_histogram_populate(histogram, array1, sizeof(array1));
    assert_int_equal(histogram->start, 0);

    // Validate the linear distribution of the elements.
    for (unsigned i = 0; i < histogram->length; i++) {
        assert_int_equal(histogram->frequencies[i], 1);
    }

    uint32_t array2[] = {0, 0, 1, 1, 8, 8, 8, 8};
    uint64_t rates2[] = {3, 3, 1, 1, 1, 1, 1, 1, 5, 1};
    huf_histogram_populate(histogram, array2, sizeof(array2));
    assert_int_equal(histogram->start, 0);

    // Validate the frequencies updates.
    for (unsigned i = 0; i < histogram->length; i++) {
        assert_int_equal(histogram->frequencies[i], rates2[i]);
    }

    huf_histogram_free(&histogram);
    assert_null(histogram);
}


static void
test_histogram_single(void **state)
{
    huf_histogram_t *histogram = NULL;

    huf_histogram_init(&histogram, 4, 10);

    uint32_t array[] = {1, 1, 1, 1, 1};
    huf_histogram_populate(histogram, array, sizeof(array));
    assert_int_equal(histogram->frequencies[1], 5);

    huf_histogram_free(&histogram);
}


// Validate the histogram start attribute updates.
static void
test_histogram_start(void **state)
{
    huf_histogram_t *histogram = NULL;

    huf_histogram_init(&histogram, 4, 10);
    assert_non_null(histogram);

    uint32_t array1[] = {4, 4, 5, 5, 5, 5, 9};
    huf_histogram_populate(histogram, array1, sizeof(array1));
    assert_true(histogram->start == 4);

    uint32_t array2[] = {1, 1, 1, 8, 8, 8};
    huf_histogram_populate(histogram, array2, sizeof(array2));
    assert_true(histogram->start == 1);

    huf_histogram_free(&histogram);
    assert_null(histogram);
}


// Validate the histogram reset.
static void
test_histogram_reset(void **state)
{
    huf_histogram_t *histogram = NULL;

    huf_histogram_init(&histogram, 4, 10);
    assert_non_null(histogram);

    uint32_t array1[] = {3, 3, 3, 3, 6, 7, 7, 1, 1, 2, 7, 7};
    uint32_t rates1[] = {0, 2, 1, 4, 0, 0, 1, 4, 0, 0};

    huf_histogram_populate(histogram, array1, sizeof(array1));
    assert_true(histogram->start == 1);

    for (unsigned i = 0; i < histogram->length; i++) {
        assert_true(histogram->frequencies[i] == rates1[i]);
    }

    huf_histogram_reset(histogram);
    assert_true(histogram->start == -1);

    for (unsigned i = 0; i < histogram->length; i++) {
        assert_true(histogram->frequencies[i] == 0);
    }

    huf_histogram_populate(histogram, array1, sizeof(array1));
    assert_true(histogram->start == 1);

    for (unsigned i = 0; i < histogram->length; i++) {
        assert_true(histogram->frequencies[i] == rates1[i]);
    }

    huf_histogram_free(&histogram);
    assert_null(histogram);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_histogram_allocation),
        cmocka_unit_test(test_histogram_populate),
        cmocka_unit_test(test_histogram_single),
        cmocka_unit_test(test_histogram_start),
        cmocka_unit_test(test_histogram_reset),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
