#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>

#include <huffman/symbol.h>


static void
test_symbol_mapping_allocation(void **state)
{
    huf_symbol_mapping_t *mapping = NULL;

    huf_symbol_mapping_init(&mapping, 10);
    assert_non_null(mapping);
    assert_true(mapping->length == 10);

    // Create a new element of the mapping.
    huf_symbol_mapping_element_t *element1 = NULL;
    huf_symbol_mapping_element_init(&element1, "1011", 4);

    assert_non_null(element1);
    assert_true(element1->length == 4);
    assert_int_equal(strcmp(element1->coding, "1011"), 0);

    // Insert an element into the mapping.
    huf_symbol_mapping_insert(mapping, 2, element1);

    huf_symbol_mapping_element_t *element2 = NULL;
    huf_symbol_mapping_get(mapping, 2, &element2);

    // Validate the elements are pointing to the same
    // data structure.
    assert_non_null(element2);
    assert_true(element1 == element2);

    huf_symbol_mapping_free(&mapping);
    assert_null(mapping);
}


static void
test_symbol_mapping_insertion(void **state)
{
    huf_symbol_mapping_t *mapping = NULL;

    huf_symbol_mapping_init(&mapping, 10);
    assert_non_null(mapping);

    // Define a few mapping elements.
    huf_symbol_mapping_element_t *element1 = NULL;
    huf_symbol_mapping_element_init(&element1, "handsomest", 10);

    huf_symbol_mapping_element_t *element2 = NULL;
    huf_symbol_mapping_element_init(&element2, "impedance", 9);

    huf_symbol_mapping_element_t *element3 = NULL;
    huf_symbol_mapping_element_init(&element3, "magnanimous", 10);

    huf_symbol_mapping_element_t *element4 = NULL;
    huf_symbol_mapping_element_init(&element4, "pitchfork", 9);

    // Insert the elements, validate the overlapping insertion.
    huf_symbol_mapping_insert(mapping, 1, element1);
    huf_symbol_mapping_insert(mapping, 1, element2);
    huf_symbol_mapping_insert(mapping, 3, element3);
    huf_symbol_mapping_insert(mapping, 4, element4);

    huf_symbol_mapping_element_t *element = NULL;
    huf_symbol_mapping_element_t *elements[] = {
        NULL, element2, NULL, element3, element4,
        NULL, NULL, NULL, NULL, NULL};

    for (unsigned i = 0; i < mapping->length; i++) {
        element = NULL;

        huf_symbol_mapping_get(mapping, i, &element);
        assert_true(element == elements[i]);
    }

    huf_symbol_mapping_free(&mapping);
    assert_null(mapping);
}


static void
test_symbol_mapping_reset(void **state)
{
    huf_symbol_mapping_t *mapping = NULL;

    huf_symbol_mapping_init(&mapping, 5);
    huf_symbol_mapping_element_t *element1 = NULL;

    for (unsigned i = 0; i < mapping->length; i++) {
        huf_symbol_mapping_element_init(&element1, "value", 5);

        huf_symbol_mapping_insert(mapping, i, element1);
    }

    // Validate all elements of the mapping.
    huf_symbol_mapping_element_t *element = NULL;
    for (unsigned i = 0; i < mapping->length; i++) {
        element = NULL;

        huf_symbol_mapping_get(mapping, i, &element);
        assert_non_null(element);

        assert_int_equal(strcmp(element->coding, "value"), 0);
        assert_true(element->length == 5);
    }

    // Reset the mapping and ensure that all elements are zeroed.
    huf_symbol_mapping_reset(mapping);
    for (unsigned i = 0; i < mapping->length; i++) {
        element = NULL;

        huf_symbol_mapping_get(mapping, i, &element);
        assert_null(element);
    }

    huf_symbol_mapping_element_t *element2 = NULL;

    // Repeat all above steps with reseted mapping.
    for (unsigned i = 0; i < mapping->length; i++) {
        huf_symbol_mapping_element_init(&element2, "attribute", 9);
        huf_symbol_mapping_insert(mapping, i, element2);
    }

    for (unsigned i = 0; i < mapping->length; i++) {
        element = NULL;

        huf_symbol_mapping_get(mapping, i, &element);
        assert_non_null(element);

        assert_int_equal(strcmp(element->coding, "attribute"), 0);
        assert_true(element->length == 9);
    }

    huf_symbol_mapping_free(&mapping);
    assert_null(mapping);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_symbol_mapping_allocation),
        cmocka_unit_test(test_symbol_mapping_insertion),
        cmocka_unit_test(test_symbol_mapping_reset),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
