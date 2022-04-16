"""This module provides classes and convenience functions for compressing and
decompressing data using Huffman compression algorithm.

The interface provided by this module is very similar to that of the :mod:`bz2` module
Note that `HuffmanCompressor`, `HuffmanDecompressor`, and `HuffmanFile` are *not*
thread-safe, so if you need to use a single instance of these classes from multiple
threads, it is necessary to protect it with a lock.
"""

__all__ = [
    "HuffmanError",
    "HuffmanFile",
    "HuffmanCompressor",
    "HuffmanDecompressor",
    "compress",
    "decompress",
]

import io
import os
from builtins import open as builtin_open

from ._C import ffi, lib


DEFAULT_BLOCK_SIZE = 131072
DEFAULT_MEM_LIMIT = 262144


class HuffmanError(Exception):
    """The exception is raised when error occurs during compression or decompression."""


def unwrap_exc(err, message):
    if err != lib.HUF_ERROR_SUCCESS:
        err_str = ffi.string(lib.huf_error_string(err))
        raise HuffmanError(f"{err_str.decode('utf-8')}. {message}")


_MODE_CLOSED = 0
_MODE_READ = 1
_MODE_WRITE = 2


class HuffmanFile(io.BufferedIOBase):
    """A file object providing transparent Huffman (de)compression.

    A HuffmanFile can act as a wrapper for an existing file object, or refer
    directly to a named file on disk.

    Note that HuffmanFile provides a *binary* file interface - data read is
    returned as bytes, and data to be written should be given as bytes.
    """

    def __init__(self, filename, mode="w", blocksize=DEFAULT_BLOCK_SIZE,
                 memlimit=DEFAULT_MEM_LIMIT):
        """Open a Huffman-compressed file.
        """
        self._fp = None
        self._mode = _MODE_CLOSED
        self._closefp = False

        if mode in ("", "r", "rb"):
            mode = "rb"
            mode_code = _MODE_READ
            self._decompressor = HuffmanDecompressor(memlimit)
        elif mode in ("w", "wb"):
            mode = "wb"
            mode_code = _MODE_WRITE
            self._compressor = HuffmanCompressor(blocksize)
        elif mode in ("x", "xb"):
            mode = "xb"
            mode_code = _MODE_WRITE
            self._compressor = HuffmanCompressor(blocksize)
        elif mode in ("a", "ab"):
            mode = "ab"
            mode_code = _MODE_WRITE
            self._compressor = HuffmanCompressor(blocksize)
        else:
            raise ValueError("Invalid mode: %r" % (mode,))

        if isinstance(filename, (str, bytes, os.PathLike)):
            self._fp = builtin_open(filename, mode)
            self._closefp = True
            self._mode = mode_code
        elif hasattr(filename, "read") or hasattr(filename, "write"):
            self._fp = filename
            self._mode = mode_code
        else:
            raise TypeError("filename must be a str, bytes, file or PathLike object")

    def close(self):
        """Flush and close the file.

        May be called more than once without error. Once the file is closed,
        any other operation on it will raise a ValueError.
        """
        if self._mode == _MODE_CLOSED:
            return
        try:
            if self._mode == _MODE_READ:
                self._decompressor.close()
                self._decompressor = None
            elif self._mode == _MODE_WRITE:
                self._fp.write(self._compressor.flush())
                self._compressor = None
        finally:
            try:
                if self._closefp:
                    self._fp.close()
            finally:
                self._fp = None
                self._closefp = False
                self._mode = _MODE_CLOSED

    @property
    def closed(self):
        """True if this file is closed."""
        return self._mode == _MODE_CLOSED

    def _check_not_closed(self):
        if self.closed:
            raise ValueError("I/O operation on closed file")

    def fileno(self):
        """Return the file descriptor for the underlying file."""
        self._check_not_closed()
        return self._fp.fileno()

    def seekable(self):
        """Return whether the file supports seeking."""
        return False

    def readable(self):
        """Return whether the file was opened for reading."""
        self._check_not_closed()
        return self._mode == _MODE_READ

    def _check_can_read(self):
        if not self.readable():
            raise io.UnsupportedOperation("File not open for reading")

    def writable(self):
        """Return whether the file was opened for writing."""
        self._check_not_closed()
        return self._mode == _MODE_WRITE

    def _check_can_write(self):
        if not self.writable():
            raise io.UnsupportedOperation("File not open for writing")

    def read(self, size=-1):
        """Read up to size uncompressed bytes from the file.

        If size is negative or omitted, read until EOF is reached.
        Returns b"" if the file is already at EOF.
        """
        self._check_can_read()
        if size < 0:
            size = io.DEFAULT_BUFFER_SIZE
        data = self._fp.read(size)
        return self._decompressor.decompress(data)

    def write(self, data):
        """Write a byte string to the file.

        Returns the number of uncompressed bytes written, which is always the
        length of data in bytes. Note that due to buffering, the file on disk
        may not reflect the data written until close() is called.
        """
        self._check_can_write()
        if isinstance(data, (bytes, bytearray)):
            length = len(data)
        else:
            # Accept any data that supports the buffer protocol.
            data = memoryview(data)
            length = data.nbytes

        compressed = self._compressor.compress(data)
        self._fp.write(compressed)
        return length


def open(filename, mode="rb", encoding=None, errors=None, newline=None):
    """Open a bzip2-compressed file in binary or text mode.

    The filename argument can be an actual filename (a str, bytes, or PathLike object),
    or an existing file object to read from or write to.

    The mode argument can be "r", "rb", "w", "wb", "x", "xb", "a" or "ab" for binary
    mode, or "rt", "wt", "xt" or "at" for text mode. The default mode is "rb".

    For binary mode, this function is equivalent to the HuffmanFile constructor:
    HuffmanFile(filename, mode). In this case, the encoding, errors and newline
    arguments must not be provided.

    For text mode, a HuffmanFile object is created, and wrapped in an io.TextIOWrapper
    instance with the specified encoding, error handling behavior, and line ending(s).
    """
    if "t" in mode and "b" in mode:
        raise ValueError("Invalid mode: %r" % (mode,))
    else:
        if encoding is not None:
            raise ValueError("Argument 'encoding' not supported in binary mode")
        if errors is not None:
            raise ValueError("Argument 'errors' not supported in binary mode")
        if newline is not None:
            raise ValueError("Argument 'newline' not supported in binary mode")

    file_mode = mode.replace("t", "")
    binary_file = HuffmanFile(filename, file_mode)

    if "t" in mode:
        return io.TextIOWrapper(binary_file, encoding, errors, newline)
    else:
        return binary_file


class MemStream:

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

    def __init__(self, blocksize=DEFAULT_BLOCK_SIZE):
        self._blocksize = blocksize
        self._flushed = False

        self.istream = MemStream(blocksize)
        self.ostream = MemStream(blocksize)

        # Initialize the encoding configuration, it's the same for the whole
        # encoding process.
        self._config = ffi.new("huf_config_t *")
        self._config.blocksize = blocksize
        self._config.reader_buffer_size = 0
        self._config.writer_buffer_size = 0
        self._config.reader = self.istream.this
        self._config.writer = self.ostream.this

    def compress(self, data):
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

        if num_blocks > 0:
            self.istream.write(data[:max_bytes])

            self._config.length = num_bytes
            err = lib.huf_encode(self._config)
            unwrap_exc(err, "Failed to encode the data")

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
    
    def flush(self):
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

    def __init__(self, memlimit=DEFAULT_MEM_LIMIT):
        self.istream = MemStream(memlimit)
        self.ostream = MemStream(memlimit)

        self._config = ffi.new("huf_config_t *")
        self._config.reader_buffer_size = 0
        self._config.writer_buffer_size = 0
        self._config.reader = self.istream.this
        self._config.writer = self.ostream.this

    def decompress(self, data):
        """Decompress data (a `bytes` object), returning uncompressed data as `bytes`.

        If data is the concatenation of multiple distinct compressed blocks, decompress
        all of these blocks, and return the concatenation of the results.
        """
        self.istream.write(data)
        self._config.length = len(self.istream)

        err = lib.huf_decode(self._config)
        unwrap_exc(err, "Failed to decode the data")

        decoding = self.ostream.getvalue()
        self.ostream.seek(0)

        return decoding

    def close(self):
        """Release the decompressor resources."""
        self._closed = True
        self.istream.close()
        self.ostream.close()


def compress(data, blocksize=DEFAULT_BLOCK_SIZE):
    """Compress *data*, returning the compressed data as a `bytes` object.

    See `HuffmanCompressor` above for a description of the *blocksize* argument.

    For incremental compression, use `HuffmanCompressor` instead.
    """
    comp = HuffmanCompressor(blocksize)
    return comp.compress(data) + comp.flush()


def decompress(data, memlimit=DEFAULT_MEM_LIMIT):
    """Decompress *data*, returning the uncompressed data as a `bytes` object.

    If *data* is the concatenation of multiple distinct compressed blocks,
    decompress all of these blocks, and return the concatenation of the results.

    See `HuffmanDecompressor` above for a description of the *memlimit*
    argument.
    """
    decomp = HuffmanDecompressor(memlimit)
    data_out = decomp.decompress(data)
    decomp.close()
    return data_out
