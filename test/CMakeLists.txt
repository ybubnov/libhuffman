find_library(cmocka_LIBRARY cmocka REQUIRED)

file(GLOB huffman_TEST_SOURCES *.c)

foreach(huffman_TEST_SOURCE ${huffman_TEST_SOURCES})
    # Extract extension fomr file name.
    get_filename_component(huffman_TEST ${huffman_TEST_SOURCE} NAME_WE)

    add_executable(${huffman_TEST} ${huffman_TEST_SOURCE})
    target_link_libraries(${huffman_TEST} ${cmocka_LIBRARY} huffman)

    # Define a new test from the source file.
    add_test(NAME ${huffman_TEST} COMMAND ${huffman_TEST})
endforeach(huffman_TEST_SOURCE)
