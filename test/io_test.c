#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>

#include <huffman/io.h>


static void
test_membuf_write(void **state)
{
    void *buf = NULL;
    huf_error_t err;
    huf_read_writer_t *mem = NULL;

    // Open a membuf with 256 bytes of capacity.
    err = huf_memopen(&mem, &buf, 256);
    assert_non_null(buf);
    assert_non_null(mem);
    assert_true(err == HUF_ERROR_SUCCESS);

    // Ensure that length in the beginning is set to 0.
    size_t len;
    err = huf_memlen(mem, &len);
    assert_true(err == HUF_ERROR_SUCCESS);
    assert_true(len == 0);

    const char *teststr = "membuf test";
    err = mem->write(mem->stream, teststr, strlen(teststr));
    assert_true(err == HUF_ERROR_SUCCESS);

    err = huf_memlen(mem, &len);
    assert_true(err == HUF_ERROR_SUCCESS);
    assert_int_equal(len, strlen(teststr));

    err = huf_memclose(&mem);
    assert_true(err == HUF_ERROR_SUCCESS);
    assert_null(mem);

    // Ensure that buffer is not freed after the close of the memclose.
    assert_string_equal(teststr, buf);
    free(buf);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_membuf_write),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
