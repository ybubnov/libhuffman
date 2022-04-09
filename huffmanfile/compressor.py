from mmap import mmap, MAP_PRIVATE
from tempfile import TemporaryFile, SpooledTemporaryFile

from ._huffmanfile import ffi, lib


HUF_1KIB_BUFFER = 1024
HUF_64KIB_BUFFER = 65536
HUF_128KIB_BUFFER = 131072
HUF_256KIB_BUFFER = 262144
HUF_512KIB_BUFFER = 524288
HUF_1MIB_BUFFER = 1048576


class HuffmanCompressor:
    """Create a new compressor object.

    This object may be used to compress data incrementally.
    """

    def __init__(self, chunk_size: int = HUF_128KIB_BUFFER):
        self._config = ffi.new("huf_config_t *")

    def compress(self, data: bytes) -> bytes:
        """Provide data to the compressor object.

        Returns a chunk of compressed data if possible, or an empty byte string 
        otherwise.
        
        When you have finished providing data to the compressor, call the `flush()`
        method to finish the compression process.
        """
        self._length += self._infile.write(data)
        return bytes()
    
    def flush(self) -> bytes:
        """Finish the compression process.

        Returns the compressed data left in internal buffers.
        The compressor object may not be used after this method is called.
        """

        return result
