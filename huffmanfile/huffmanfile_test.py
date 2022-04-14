from string import printable

from .huffmanfile import compress, decompress


def test_compress_decompress():
    input_bytes = printable.encode("utf-8")
    compressed = compress(input_bytes, memlimit=1024)
    uncompressed = decompress(compressed, memlimit=1024)
    assert uncompressed == input_bytes
