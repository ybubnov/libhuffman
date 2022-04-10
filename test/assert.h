#ifndef INCLUDE_huffman_assert_h__
#define INCLUDE_huffman_assert_h__


#define assert_ok(statement) \
    assert_int_equal((statement), HUF_ERROR_SUCCESS) \


#endif // INCLUDE_huffman_assert_h
