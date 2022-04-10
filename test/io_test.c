#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>

#include <huffman/io.h>
#include "assert.h"


static void
test_membuf_write(void **state)
{
    void *buf = NULL;
    huf_read_writer_t *mem = NULL;

    // Open a membuf with 256 bytes of capacity.
    assert_ok(huf_memopen(&mem, &buf, 256));
    assert_non_null(buf);
    assert_non_null(mem);

    // Ensure that length in the beginning is set to 0.
    size_t len;
    assert_ok(huf_memlen(mem, &len));
    assert_int_equal(len, 0);

    const char *teststr = "membuf test";
    assert_ok(mem->write(mem->stream, teststr, strlen(teststr)));

    assert_ok(huf_memlen(mem, &len));
    assert_int_equal(len, strlen(teststr));

    assert_ok(huf_memclose(&mem));
    assert_null(mem);

    // Ensure that buffer is not freed after the close of the memclose.
    assert_string_equal(teststr, buf);
    free(buf);
}


static void
test_membuf_realloc(void **state)
{
    void *buf = NULL;
    size_t cap = 0;
    huf_read_writer_t *mem = NULL;

    assert_ok(huf_memopen(&mem, &buf, 2));

    assert_ok(mem->write(mem->stream, "01", 2));
    assert_ok(huf_memcap(mem, &cap));
    assert_int_equal(cap, 2);

    void *prev_buf = buf;
    assert_ok(mem->write(mem->stream, "23456789", 8));
    assert_ok(huf_memcap(mem, &cap));

    // The point to the backend memory should be updated, since the mem
    // allocated more space to hold a new piece of data.
    assert_int_equal(cap, 16);
    assert_ptr_not_equal(prev_buf, buf);

    assert_ok(huf_memclose(&mem));
    free(buf);
}


static void
test_membuf_read(void **state)
{
    void *buf = NULL;
    huf_read_writer_t *mem = NULL;

    assert_ok(huf_memopen(&mem, &buf, 8));
    assert_ok(mem->write(mem->stream, "abcd", 4));

    char dest[8] = {0};
    size_t destsz = 8;

    assert_ok(mem->read(mem->stream, dest, &destsz));
    assert_int_equal(destsz, 4);

    assert_string_equal(dest, "abcd");

    // Read again from the buffer, that should be empty by now.
    destsz = 8;
    assert_ok(mem->read(mem->stream, dest, &destsz));
    assert_int_equal(destsz, 0);
    assert_string_equal(dest, "abcd");

    assert_ok(huf_memclose(&mem));
    free(buf);
}


int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_membuf_write),
        cmocka_unit_test(test_membuf_realloc),
        cmocka_unit_test(test_membuf_read),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
