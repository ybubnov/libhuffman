from string import printable

import pytest

from . import huffmanfile


def test_compress_decompress():
    data = b"a"*1000
    c = huffmanfile.compress(data)
    d = huffmanfile.decompress(c)
    assert d == data


def test_decompress_corrupted():
    with pytest.raises(huffmanfile.HuffmanError):
        data = b'\x08\x00\x00\x00\x00\x00\x00\x00\x02\x00'
        huffmanfile.decompress(data)


def test_compress_incremental():
    def gen_data(parts=10, partsize=1000):
        for _ in range(parts):
            yield b"z" * partsize

    comp = huffmanfile.HuffmanCompressor()
    out = bytes()
    data = bytes()
    for data_part in gen_data():
        out += comp.compress(data_part)
        data += data_part

    out += comp.flush()
    assert huffmanfile.decompress(out) == data
