#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>

#include <huffman.h>


#define assert_ok(statement) \
    assert_int_equal((statement), HUF_ERROR_SUCCESS) \


static void
test_encoder(void **state)
{
    void *bufin, *bufout = NULL;
    huf_error_t err;

    huf_read_writer_t *input = NULL;
    huf_read_writer_t *output = NULL;

    assert_ok(huf_memopen(&input, &bufin, 128));
    assert_ok(huf_memopen(&output, &bufout, 2048));

    huf_config_t encoder_config = {
        .length = 10,
        .reader_buffer_size = 128,
        .writer_buffer_size = 2048,
        .reader = input,
        .writer = output,
    };

    assert_ok(input->write(input->stream, "0123456789", 10));
    assert_ok(huf_encode(&encoder_config));

    size_t output_len = 0;
    assert_ok(huf_memlen(output, &output_len));
    assert_true(output_len > 0);

    assert_ok(huf_memclose(&input));
    assert_ok(huf_memclose(&output));
}


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_encoder),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
