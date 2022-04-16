# libhuffman - The Huffman coding library

The Huffman library is a simple, pure C library for encoding and decoding data using a
frequency-sorted binary tree.
The implementation of this library is pretty straightforward, additional information
regarding Huffman coding could be gained from the [Wikipedia](https://en.wikipedia.org/wiki/Huffman_coding).

## Installation

The build mechanism of the library is based on the [CMake](https://cmake.org) tool, so
you could easily install it on your distribution in the following way:
```bash
$ sudo cmake install
```

By default the command above install the library into `/usr/local/lib` and all
required headers into `/usr/local/include`. The installation process is organized
using CMake. Just create a new directory `build` and generate required makefiles:
```bash
$ mkdir -p build
$ cmake ..
```

After that run the `install` target:
```bash
$ make install
```

## Usage

At this moment only two methods for encoding and decoding are available. To encode the
data from the file descriptor at the first step the configuration should be defined:
```c
huf_read_writer_t *file_in;
huf_fdopen(&file_in, 0) // Read from standard input.

void *buf = NULL;
huf_read_writer_t *mem_out;
huf_memopen(&mem_out, &buf, HUF_1MIB_BUFFER);

huf_config_t config = {
   .reader = file_in,
   .writer = mem_out,
   .length = length,
   .blocksize = HUF_64KIB_BUFFER,
   .reader_buffer_size = HUF_128KIB_BUFFER,
   .writer_buffer_size = HUF_128KIB_BUFFER,
};
```

- `fdin` - input file descriptor.
- `fdout` - output file descriptor.
- `length` - length of the data in bytes to encode.
- `blocksize` - the length of each chunk in bytes (instead of reading the file twice `libhuffman` is reading and encoding the data by blocks).
- `reader_buffer_size` - this is opaque reader buffer size in bytes, if the buffer size is set to zero, all reads will be unbuffered.
- `writer_buffer_size` - this is opaque writer buffer size ib bytes, if the buffer size is set to zero, all writes will be unbuffered.

After the configuration is created, it could be passed right to the required function (to encode or to decode):
```c
huf_error_t err = huf_encode(&config);
printf("%s\n", huf_error_string(err));
```

## Python Bindings

Python bindings for `libhuffman` library are distributed as PyPI package, to install
that package, execute the following command:
```sh
pip install huffmanfile
```

You can use the `libhuffman` for performant compression and decompression of Huffman
encoding. The interface of the Python library is similar to the interface of the
[`bz2`](https://docs.python.org/3/library/bz2.html) and
[`lzma`](https://docs.python.org/3/library/lzma.html) packages from Python's standard
library.

### Examples of usage

Using `compress()` and `decompress()` to demonstrate round-trip compression:
```py
>>> import huffmanfile
>>> data = b"""\
... Donec rhoncus quis sapien sit amet molestie. Fusce scelerisque vel augue
... nec ullamcorper. Nam rutrum pretium placerat. Aliquam vel tristique lorem,
... sit amet cursus ante. In interdum laoreet mi, sit amet ultrices purus
... pulvinar a. Nam gravida euismod magna, non varius justo tincidunt feugiat.
... Aliquam pharetra lacus non risus vehicula rutrum. Maecenas aliquam leo
... felis. Pellentesque semper nunc sit amet nibh ullamcorper, ac elementum
... dolor luctus. Curabitur lacinia mi ornare consectetur vestibulum.""" * 3
>>> c = huffmanfile.compress(data)
>>> len(data) / len(c)  # Data compression ratio
1.1872037914691944
>>> d = huffmanfile.decompress(c)
>>> data == d  # Check equality to original object after round-trip
True
```

Using `HuffmanCompressor` for incremental compression:
```py
>>> import huffmanfile
>>> def gen_data(parts=10, partsize=1000):
...     for _ in range(parts):
...         yield b"z" * partsize
...
>>> comp = huffmanfile.HuffmanCompressor()
>>> out = b""
>>> for data_part in gen_data():
...     # Provide data to the compressor object
...     out += comp.compress(data_part)
...
>>> # Finish the compression process.  Call this once you have
>>> # finished providing data to the compressor.
>>> out += comp.flush()
```

The example above uses a very "nonrandom" stream of data (a stream of b”z” parts).
Random data tends to compress poorly, while ordered, repetitive data usually yields a
high compression ratio.

## License

The Huffman library is distributed under MIT license, therefore you are free to do with
code whatever you want. See the [LICENSE](LICENSE) file for full license text.
