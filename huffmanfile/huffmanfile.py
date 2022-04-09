import os


class HuffmanFile:

    def __init__(self, filename, mode="w"):
        """Open a Huffman-compressed file.

        """
        if isinstance(filename, (str, bytes, os.PathLike)):
            self._fp = open(filename, mode)
            self._mode = mode 
        else:
            raise TypeError("filename must be a str, bytes, file or PathLike object")
