#!/usr/bin/env python

from typing import List

import cffi


def make_library_prototypes(headers: List[str]) -> str:
    source_code: str = ""

    for header in headers:
        with open(f"include/{header}") as header_file:
            include_statement: bool = False
            for line in header_file.readlines():
                # Exclude derictives as long as they are not supported by FFI.
                if line.startswith("#define CFFI"):
                    include_statement = True
                    continue
                if line.startswith("#undef CFFI"):
                    include_statement = False
                if include_statement:
                    source_code += line
    return source_code


def make_library_header(headers: List[str]) -> str:
    return "\n".join(f"#include <{header}>" % header for header in headers),


headers = [
    "huffman/errors.h",
    "huffman/io.h",
    "huffman/config.h",
    "huffman/common.h",
    "huffman/decoder.h",
    "huffman/encoder.h",
    "huffman/bufio.h",
    "huffman/histogram.h",
    "huffman/malloc.h",
    "huffman/symbol.h",
    "huffman/sys.h",
    "huffman/tree.h",
]

sources = [
    "src/bufio.c",
    "src/config.c",
    "src/decoder.c",
    "src/encoder.c",
    "src/errors.c",
    "src/histogram.c",
    "src/io.c",
    "src/malloc.c",
    "src/symbol.c",
    "src/tree.c",
]


ffibuilder = cffi.FFI()
ffibuilder.set_source(
    "huffmanlib._huffmanlib",
    make_library_header(headers),
    include_dirs=["include"],
    sources=sources,
)
ffibuilder.cdef(make_library_prototypes(headers))


if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
