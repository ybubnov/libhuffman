#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>

#include <huffman.h>
#include "assert.h"
#include <stdio.h>


static void
test_encode_nobuffer(void **state)
{
    void *bufin, *bufout = NULL;
    huf_read_writer_t *input, *output = NULL;

    assert_ok(huf_memopen(&input, &bufin, 128));
    assert_ok(huf_memopen(&output, &bufout, 128));

    // Encode only a single symbol.
    assert_ok(input->write(input->stream, "1", 1));

    huf_config_t config = {
        .blocksize = 256,
        .length = 1,
        .reader = input,
        .writer = output,
    };

    assert_ok(huf_encode(&config));
    
    size_t encoding_len = 0;
    assert_ok(huf_memlen(output, &encoding_len));
    assert_int_equal(encoding_len, 21);

    assert_ok(huf_memclose(&input));
    assert_ok(huf_memclose(&output));

    assert_null(input);
    assert_null(output);

    free(bufin);
    free(bufout);
}


static void
test_encode_decode(void **state)
{
    void *bufin, *bufout = NULL;

    huf_read_writer_t *input = NULL;
    huf_read_writer_t *output = NULL;

    assert_ok(huf_memopen(&input, &bufin, 128));
    assert_ok(huf_memopen(&output, &bufout, 2048));

    huf_config_t config = {
        .length = 10,
        .reader_buffer_size = 128,
        .writer_buffer_size = 128,
        .reader = input,
        .writer = output,
    };

    assert_ok(input->write(input->stream, "0123456789", 10));
    assert_ok(huf_encode(&config));

    size_t encoding_len = 0;
    assert_ok(huf_memlen(output, &encoding_len));
    assert_true(encoding_len > 0);

    // Switch reader and writer to decode encoded information.
    config.reader = output;
    config.writer = input;
    config.length = encoding_len;

    printf("ENCODED %d bytes!\n", encoding_len);
    assert_ok(huf_decode(&config));

    printf("DECODED!\n");
    // Ensure that decoding result is the same as the encoded string.
    // Put an extra character for a null-terminated string comparison.
    char result[11] = {0};
    size_t result_len = 10;
    assert_ok(input->read(input->stream, result, &result_len));
    assert_int_equal(result_len, 10);
    assert_string_equal(result, "0123456789");

    assert_ok(huf_memclose(&input));
    assert_ok(huf_memclose(&output));

    free(bufin);
    free(bufout);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_encode_nobuffer),
        cmocka_unit_test(test_encode_decode),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
