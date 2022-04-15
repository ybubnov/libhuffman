from string import printable

import pytest

from .huffmanfile import compress, decompress, HuffmanError


def test_compress_decompress():
    input_bytes = printable.encode("utf-8")
    compressed = compress(input_bytes, memlimit=1024)
    uncompressed = decompress(compressed, memlimit=1024)
    assert uncompressed == input_bytes


def test_decompress_corrupted():
    with pytest.raises(HuffmanError):
        input_bytes = b'\x08\x00\x00\x00\x00\x00\x00\x00\x02\x00'
        decompress(input_bytes)
