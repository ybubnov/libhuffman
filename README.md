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

### Encoding

To encode the data, use either a file stream `huf_fdopen` or `huf_memopen` to use
an in-memory stream. Consider the following example, where the input is a memory
stream and output of the encoder is also memory buffer of 1MiB size.
```c
void *bufin, *bufout = NULL;
huf_read_writer_t *input, *output = NULL;

// Allocate the necessary memory.
huf_memopen(&input, &bufin, HUF_1MIB_BUFFER);
huf_memopen(&output, &bufout, HUF_1MIB_BUFFER);

// Write the data for encoding to the input.
size_t input_len = 10;
input->write(input->strea, "0123456789", input_len);
```

Create a configuration used to encode the input string using Huffman algorithm:
```c
huf_config_t config = {
   .reader = input,
   .length = input_len,
   .writer = output,
   .blocksize = HUF_64KIB_BUFFER,
};

huf_error_t err = huf_encode(&config);
printf("%s\n", huf_error_string(err));
```

- `reader` - input ready for the encoding.
- `writer` - output for the encoded data.
- `length` - length of the data in bytes to encode.
- `blocksize` - the length of each chunk in bytes (instead of reading the file twice
`libhuffman` reads and encodes data by blocks).
- `reader_buffer_size` - this is opaque reader buffer size in bytes, if the buffer size
is set to zero, all reads will be unbuffered.
- `writer_buffer_size` - this is opaque writer buffer size ib bytes, if the buffer size
is set to zero, all writes will be unbuffered.

After the encoding, the output memory buffer could be automatically scaled to fit all
necessary encoded bytes. To retrieve a new length of the buffer, use the following:
```c
size_t out_size = 0;
huf_memlen(output, &out_size);

// The data is accessible through the `bufout` variable or using `read` function:
uint8_t result[10] = {0};
size_t result_len = 10;

// result_len is inout parameter, and will contain the length of encoding
// after the reading from the stream.
output->read(output->stream, result, &result_len);
```

### Decoding

Decoding is similar to the encoding, except that reader attribute of the configuration
should contain the data used to decode:
```c
input->write(input->stream, decoding, decoding_len);

huf_config_t config = {
    .reader = input,
    .length = input_len,
    .writer = output,
    .blockize = HUF_64KIB_BUFFER,
};


// After the decoding the original data will be writter to the `output`.
huf_decode(&config);
```

## Resource Deallocation

Once the processing of the encoding is completed, consider freeing the allocated memory:
```c
// This does not free underlying buffer, call free for the buffer.
huf_memclose(&mem_out);

free(buf);
```

For more examples, please, refer to the [`tests`](tests) directory.

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
with huffmanfile.open("file.hm", "w") as f:
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
