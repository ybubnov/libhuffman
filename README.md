# libhuffman - The Huffman coding library

The Huffman library is a simple, pure C thread-safe library for encoding and decoding data using a frequency-sorted binary tree.
The implementation of this library is pretty straightforward, additional information regarding Huffman coding could be gained from the Wikipedia [page](https://en.wikipedia.org/wiki/Huffman_coding).

## Installation

The build mechanism of the library is based on the [scons](http://scons.org/) tool, so you could easily install it on your distribution in the following way:
```bash
# scons install
```

By default the command above install the library into ```/usr/local/lib``` and all required headers into ```/usr/local/include```.
To remove the installed library, you could type the following command:
```bash
# scons uninstall
```

## Usage

At this moment only two methods for econding and deconding appropriately are available.
To encode the data by the file descriptor at the first step the configuration should be defined:
```c
huf_config_t config = {
   .reader = fdin,
   .writer = fdout,
   .length = length,
   .chunk_size = HUF_64KIB_BUFFER,
   .reader_buffer_size = HUF_128KIB_BUFFER,
   .writer_buffer_size = HUF_128KIB_BUFFER,
};
```

- ```fdin``` - input file descriptor.
- ```fdout``` - output file descriptor.
- ```length``` - length of the data in bytes to encode.
- ```chunk_size``` - the length of each chunk in bytes (instead of reading the file twice ```libhuffman``` is reading and encoding the data by chunks).
- ```reader_buffer_size``` - this is opaque reader buffer size in bytes, if the buffer size set to zero it will be defaulted to 64 KiB.
- ```writer_buffer_size``` - this is opaque writer buffer size ib bytes, will be defaulted as like as a reader buffer.

After the configuration is crafted, it could be passed right to the required function (to encode or to decode):
```c
huf_error_t err = huf_encode(&config);
printf("%s\n", huf_error_string(err));
```

## License

The Huffman library is distributed under MIT license, therefore you are free to do with code whatever you want. See the [LICENSE](LICENSE) file for full license text.
