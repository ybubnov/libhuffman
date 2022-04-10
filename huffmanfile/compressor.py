import io

from ._huffmanfile import ffi, lib


_1KIB = 1024
_64KIB = 65536
_128KIB = 131072
_256KIB = 262144
_512KIB = 524288
_1MIB = 1048576


class IOStream:

    __slots__ = ["_len_ptr", "_buf_ptr", "_ptr"]

    def __init__(self, capacity):
        self._len_ptr = ffi.new("size_t *")

        # Create pointer for membuf backend memory, it's much easier to
        # access data through these pointers rather than copying content.
        self._buf_ptr = ffi.new("void **")

        # Open memory buffers for reading and writing.
        self._ptr = ffi.new("huf_read_writer_t **")

        err = lib.huf_memopen(self._ptr, self._buf_ptr, capacity)

    @property
    def this(self):
        return self._ptr[0]

    def close(self):
        lib.huf_memclose(self._ptr)

    def getvalue(self):
        buf = ffi.cast("char *", self._buf_ptr[0])
        return ffi.buffer(buf, len(self))[:]

    def write(self, data):
        err = self.this.write(self.this.stream, data, len(data))

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
        lib.huf_memrewind(self.this)

    def __len__(self):
        err = lib.huf_memlen(self.this, self._len_ptr)
        return self._len_ptr[0]



class HuffmanCompressor:
    """Create a new compressor object.

    This object may be used to compress data incrementally.
    """

    def __init__(self, blocksize: int = 131072):
        self._blocksize = blocksize

        self.istream = IOStream(blocksize * 2)
        self.ostream = IOStream(blocksize * 2)

        # Initialize the encoding configuration, it's the same for the whole
        # encoding process.
        self._config = ffi.new("huf_config_t *")
        self._config.reader_buffer_size = max(blocksize * 2, _512KIB)
        self._config.writer_buffer_size = max(blocksize * 2, _512KIB)
        self._config.reader = self.istream.this
        self._config.writer = self.ostream.this

    def compress(self, data: bytes) -> bytes:
        """Provide data to the compressor object.

        Returns a block of compressed data if possible, or an empty byte string 
        otherwise.
        
        When you have finished providing data to the compressor, call the `flush()`
        method to finish the compression process.
        """
        num_bytes = len(data)
        num_blocks = num_bytes // self._blocksize
        max_bytes = num_blocks * self._blocksize

        if max_bytes >= self._blocksize:
            self.istream.write(data[:max_bytes])
            print(f"input_len = {len(self.istream)}")

            self._config.length = max_bytes
            err = lib.huf_encode(self._config)

            print(f"output_len = {len(self.ostream)}")
            return self.ostream.getvalue()

        return bytes()
    
    def flush(self) -> bytes:
        """Finish the compression process.

        Returns the compressed data left in internal buffers.
        The compressor object may not be used after this method is called.
        """

        self.istream.close()
        self.ostream.close()

        return bytes()
