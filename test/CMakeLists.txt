include_directories(AFTER /usr/local/include)

find_library(cmocka_LIBRARY cmocka REQUIRED)

file(GLOB huffman_TEST_SOURCES *.c)
add_definitions(-std=c99)

find_program(valgrind_PROGRAM valgrind)
set(valgrind_PROGRAM_OPTIONS "--leak-check=full")
set(valgrind_COMMAND ${valgrind_PROGRAM} ${valgrind_PROGRAM_OPTIONS})

foreach(huffman_TEST_SOURCE ${huffman_TEST_SOURCES})
    # Extract extension fomr file name.
    get_filename_component(huffman_TEST ${huffman_TEST_SOURCE} NAME_WE)

    add_executable(${huffman_TEST} ${huffman_TEST_SOURCE})
    target_link_libraries(${huffman_TEST} ${cmocka_LIBRARY} huffman)

    # Define a new test from the source file.
    add_test(NAME ${huffman_TEST} COMMAND ${huffman_TEST})

    # Define a valgrind memory leak checking.
    if(valgrind_PROGRAM)
        add_test(NAME memcheck_${huffman_TEST} COMMAND ${valgrind_COMMAND}
            ${CMAKE_CURRENT_BINARY_DIR}/${huffman_TEST})
    endif()
endforeach(huffman_TEST_SOURCE)
