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

Reading in a compressed file:
```py
import huffmanfile
with huffmanfile.open("file.hm") as f:
    file_content = f.read()
```

Creating a compressed file:
```py
import huffmanfile
data = b"Insert Data Here"
with huffmanfile.open(file.hm", "w") as f:
    f.write(data)
```

Compressing data in memory:
```py
import huffmanfile
data_in = b"Insert Data Here"
data_out = huffmanfile.compress(data_in)
```

Incremental compression:
```py
import huffmanfile
hfc = huffmanfile.HuffmanCompressor()
out1 = hfc.compress(b"Some data\n")
out2 = hfc.compress(b"Another piece of data\n")
out3 = hfc.compress(b"Even more data\n")
out4 = hfc.flush()
# Concatenate all the partial results:
result = b"".join([out1, out2, out3, out4])
```

Note, random data tends to compress poorly, while ordered, repetitive data usually
yields a high compression ratio.

## License

The Huffman library is distributed under MIT license, therefore you are free to do with
code whatever you want. See the [LICENSE](LICENSE) file for full license text.
