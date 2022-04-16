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


def test_write_file(tmp_path):
    data = """\
    Donec rhoncus quis sapien sit amet molestie. Fusce scelerisque vel augue
    nec ullamcorper. Nam rutrum pretium placerat. Aliquam vel tristique lorem,
    sit amet cursus ante. In interdum laoreet mi, sit amet ultrices purus
    pulvinar a. Nam gravida euismod magna, non varius justo tincidunt feugiat.
    Aliquam pharetra lacus non risus vehicula rutrum. Maecenas aliquam leo
    felis. Pellentesque semper nunc sit amet nibh ullamcorper, ac elementum
    dolor luctus. Curabitur lacinia mi ornare consectetur vestibulum."""

    filename = tmp_path / "archive.hm"

    with huffmanfile.open(filename, "wt") as f:
        f.write(data)
    with huffmanfile.open(filename, "rt") as f:
        content = f.read()

    assert content == data
