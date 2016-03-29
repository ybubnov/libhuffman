cmake_minimum_required(VERSION 2.8)

project(huffman C)

set(huffman_LIBRARY_VERSION "0.0.1")
set(huffman_LIBRARY_SOVERSION "0")


include_directories(include)
include_directories(include/huffman)

aux_source_directory(src huffman_SOURCES)

add_subdirectory(test)

add_library(huffman SHARED ${huffman_SOURCES})
set_target_properties(huffman PROPERTIES VERSION ${huffman_LIBRARY_VERSION})
set_target_properties(huffman PROPERTIES SOVERSION ${huffman_LIBRARY_SOVERSION})

add_definitions(-std=c99)


install(DIRECTORY include/huffman DESTINATION huffman)
install(FILES include/huffman.h DESTINATION include)
install(TARGETS huffman DESTINATION lib)
