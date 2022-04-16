#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>
#include <stdio.h>

#include <huffman.h>
#include "assert.h"


static void
test_decoder_corrupted(void **state)
{
    void *bufin, *bufout = NULL;
    huf_read_writer_t *input, *output = NULL;

    assert_ok(huf_memopen(&input, &bufin, 128));
    assert_ok(huf_memopen(&output, &bufout, 128));

    huf_config_t config = {
        .length = 0,
        .reader_buffer_size = 128,
        .writer_buffer_size = 128,
        .reader = input,
        .writer = output,
    };

    // Do no write anything to the input and attempt to decode.
    // This configuration should be successful.

    assert_ok(huf_decode(&config));

    size_t len = 0;
    assert_ok(huf_memlen(output, &len));
    assert_int_equal(len, 0);

    // Now write a piece of "arbitrary" data to the input stream and
    // ensure that an error is returned after attempt to decoding
    // instead of a segmentation fault.
    const uint8_t arbitrary_data[] = {
        10, 10, 10, 10, 10, 10, 10, 10, 10, 10
    };
    assert_ok(input->write(input->stream, arbitrary_data, sizeof(arbitrary_data)));

    config.length = sizeof(arbitrary_data);
    assert_int_equal(huf_decode(&config), HUF_ERROR_BTREE_OVERFLOW);

    // Reset the input, and write the acceptable length of the Huffman tree
    // but in the new case, the tree itself, is corrupted.
    const uint8_t truncated_data[] = {
        8, 0, 0, 0, 0, 0, 0, 0, 8, 0, 10, 10, 10, 10
    };
    assert_ok(huf_memrewind(input));
    assert_ok(input->write(input->stream, truncated_data, sizeof(truncated_data)));

    // The tree consists of 8 elements (according to 'truncated_data') encoding,
    // but this request cannot be satisfied by a buffered IO, since the memory
    // stream does not provide enough bytes.
    assert_int_equal(huf_decode(&config), HUF_ERROR_READ_WRITE);

    // Test, that corrupted block returns an error instead of segmentation fault.
    const int16_t corrupted_data[] = {
        8, 0, 0, 0, // block size
        3, // number of tree elements
        0, -1, -1, // Huffman tree, these elements are not enough to encode the block.
        1, 2, 3, // there are only 6 bytes instead of 8 (block size).
    };

    assert_ok(huf_memrewind(input));
    assert_ok(input->write(input->stream, corrupted_data, sizeof(corrupted_data)));

    config.length = sizeof(corrupted_data);
    assert_int_equal(huf_decode(&config), HUF_ERROR_BTREE_CORRUPTED);

    assert_ok(huf_memclose(&input));
    assert_ok(huf_memclose(&output));

    free(bufin);
    free(bufout);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_decoder_corrupted),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
