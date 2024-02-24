cmake_minimum_required(VERSION 3.5)

project(huffman C)
enable_testing()

set(huffman_LIBRARY_VERSION "1.0.2")
set(huffman_LIBRARY_SOVERSION "1")


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
