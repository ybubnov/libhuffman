"""This module provides classes and convenience functions for compressing and
decompressing data using Huffman compression algorithm.

The interface provided by this module is very similar to that of the :mod:`bz2` module
Note that :class:`HuffmanCompressor`, :class:`HuffmanDecompressor`, and
:class:`HuffmanFile` are *not* thread-safe, so if you need to use a single instance
of these classes from multiple threads, it is necessary to protect it with a lock.
"""

__all__ = [
    "HuffmanError",
    "HuffmanFile",
    "HuffmanCompressor",
    "HuffmanDecompressor",
]

import io
import os

from ._huffmanfile import ffi, lib


class HuffmanError(Exception):
    """The exception is raised when error occurs during compression or decompression."""


def unwrap_exc(err, message):
    if err != lib.HUF_ERROR_SUCCESS:
        err_str = lib.huf_error_string(err)
        raise HuffmanError(f"{err_str}. {message}")


class HuffmanFile:

    def __init__(self, filename, mode="w"):
        """Open a Huffman-compressed file.

        """
        if isinstance(filename, (str, bytes, os.PathLike)):
            self._fp = open(filename, mode)
            self._mode = mode 
        else:
            raise TypeError("filename must be a str, bytes, file or PathLike object")


class IOStream:

    __slots__ = ["_len_ptr", "_buf_ptr", "_ptr"]

    def __init__(self, capacity):
        self._len_ptr = ffi.new("size_t *")

        # Create pointer for membuf backend memory, it's much easier to
        # access data through these pointers rather than copying content.
        #
        # Open memory buffers for reading and writing.
        self._buf_ptr = ffi.new("void **")
        self._ptr = ffi.new("huf_read_writer_t **")

        err = lib.huf_memopen(self._ptr, self._buf_ptr, capacity)
        unwrap_exc(err, f"Failed to allocate memory stream of {capacity} bytes long")

    @property
    def this(self):
        return self._ptr[0]

    def close(self):
        err = lib.huf_memclose(self._ptr)
        unwrap_exc(err, "Failed to close memory stream")

    def getvalue(self):
        buf = ffi.cast("char *", self._buf_ptr[0])
        return ffi.buffer(buf, len(self))[:]

    def write(self, data):
        err = self.this.write(self.this.stream, data, len(data))
        unwrap_exc(err, "Failed to write data to the memory stream")

    def seek(self, offset, whence=io.SEEK_SET):
        if whence != io.SEEK_SET:
            raise ValueError(
                "Seek on in-memory stream allows only io.SEEK_SET; got "
                f"{whence}, which is not supported"
            )
        if offset != 0:
            raise ValueError(
                "Seek on in-memory stream allows only rewinds; got "
                f"offset = {offset} which is not supported"
            )
        err = lib.huf_memrewind(self.this)
        unwrap_exc(err, "Failed to rewind memory stream")

    def __len__(self):
        err = lib.huf_memlen(self.this, self._len_ptr)
        unwrap_exc(err, "Failed to retrieve length of the memory stream")
        return self._len_ptr[0]


class HuffmanCompressor:
    """Create a new compressor object.

    This object may be used to compress data incrementally.
    """

    def __init__(self, blocksize, memlimit):
        self._blocksize = blocksize
        self._flushed = False

        self.istream = IOStream(blocksize)
        self.ostream = IOStream(blocksize)

        # Initialize the encoding configuration, it's the same for the whole
        # encoding process.
        self._config = ffi.new("huf_config_t *")
        self._config.blocksize = blocksize
        self._config.reader_buffer_size = memlimit
        self._config.writer_buffer_size = memlimit
        self._config.reader = self.istream.this
        self._config.writer = self.ostream.this

    def compress(self, data: bytes) -> bytes:
        """Provide data to the compressor object.

        Returns a block of compressed data if possible, or an empty byte string 
        otherwise.
        
        When you have finished providing data to the compressor, call the `flush()`
        method to finish the compression process.
        """
        encoding = bytes()
        if self._flushed:
            return encoding()

        buf_bytes = len(self.istream)

        # Calculate the amount of bytes ready for encoding considering the content
        # of the buffer in the input stream. If at least one block is available for
        # encoding, run the encoder, put remaining bytes to the buffer, until the
        # next `compress` or `flush` call.
        all_bytes = buf_bytes + len(data)
        num_blocks = all_bytes // self._blocksize
        num_bytes = num_blocks * self._blocksize

        # Calculate how much data is required to take from the input bytes and what
        # to write the input stream until the next call.
        max_bytes = num_bytes - buf_bytes
        rem_bytes = len(data) - max_bytes
        print(f"data_bytes={len(data)}")
        print(f"num_blocks={num_blocks}, max_bytes={max_bytes}, rem_bytes={rem_bytes}")

        if num_blocks > 0:
            self.istream.write(data[:max_bytes])
            print(f"input_len = {len(self.istream)}")

            self._config.length = num_bytes
            err = lib.huf_encode(self._config)
            unwrap_exc(err, "Failed to encode the data")

            print(f"output_len = {len(self.ostream)}")
            encoding = self.ostream.getvalue()

            # Reset input stream, to write next time a new portion of data. Reset
            # output stream to discard content in the buffer.
            self.istream.seek(0)
            self.ostream.seek(0)

        # Write the remaining bytes to the input buffer, so the encoding could
        # continue. The istream can grow as large as necessary to fit all the
        # remaining bytes.
        if rem_bytes > 0:
            self.istream.write(data[max_bytes:max_bytes+rem_bytes])

        return encoding
    
    def flush(self) -> bytes:
        """Finish the compression process.

        Returns the compressed data left in internal buffers. The compressor object
        may not be used after this method is called.
        """
        encoding = bytes()

        if self._flushed:
            return encoding

        self._config.length = len(self.istream)

        if self._config.length > 0:
            err = lib.huf_encode(self._config)
            unwrap_exc(err, "Failed to encode the data")
            encoding = self.ostream.getvalue()

        self.istream.close()
        self.ostream.close()
        self._flushed = True

        return encoding


class HuffmanDecompressor:
    """Create a new decompressor object.

    This object may be used to decompress data incrementally.
    """

    def __init__(self, memlimit: int = 131072):
        self.istream = IOStream(memlimit)
        self.ostream = IOStream(memlimit)

        self._config = ffi.new("huf_config_t *")
        self._config.reader_buffer_size = memlimit * 2
        self._config.writer_buffer_size = memlimit * 2
        self._config.reader = self.istream.this
        self._config.writer = self.ostream.this

    def decompress(self, data: bytes) -> bytes:
        """Decompress data (a bytes object), returning uncomressed data as bytes object.

        If data is the concatenation of multiple distinct compressed blocks, decompress
        all of these blocks, and return the concatenation of the results.
        """

        self.istream.write(data)
        self._config.length = len(self.istream)
        print(f"input_len={self._config.length}")
        err = lib.huf_decode(self._config)
        unwrap_exc(err, "Faild to decode the data")

        decoding = self.ostream.getvalue()
        self.ostream.seek(0)

        return decoding


def compress(data, blocksize=131072, memlimit=262144):
    """Compress *data* (a :class:`bytes` object), returning the compressed data as a
    :class:`bytes` object.

    See :class:`HuffmanCompressor` above for a description of the *blocksize* and
    *memlimit* arguments.

    For incremental compression, use :class:`HuffmanCompressor` instead.
    """
    comp = HuffmanCompressor(blocksize, memlimit)
    return comp.compress(data) + comp.flush()


def decompress(data, memlimit=262144):
    """Decompress *data* (a :class:`bytes` object), returning the uncompressed data
    as a :class:`bytes` object.

    If *data* is the concatenation of multiple distinct compressed blocks,
    decompress all of these blocks, and return the concatenation of the results.

    See :class:`HuffmanDecompressor` above for a description of the *memlimit*
    argument.
    """
    decomp = HuffmanDecompressor(memlimit)
    return decomp.decompress(data)
