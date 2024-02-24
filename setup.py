from pathlib import Path
from setuptools import setup


setup(
    name="huffmanfile",
    version="1.0.3",

    long_description=Path("README.md").read_text(),
    long_description_content_type="text/markdown",
    description="Python bindings for libhuffman",

    url="https://github.com/ybubnov/libhuffman",
    author="Yasha Bubnov",
    author_email="girokompass@gmail.com",

    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Topic :: Software Development :: Libraries",
        "Topic :: System :: Archiving :: Compression",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "License :: OSI Approved :: Apache Software License",
        "License :: OSI Approved :: MIT License",
    ],

    keywords=["huffman", "encoding", "decoding", "compression"],
    package_dir={"huffmanfile": "huffmanfile"},
    packages=["huffmanfile"],
    include_package_data=True,
    package_data={"build_tools": ["setup_ffi.py"]},

    tests_require=["pytest"],
    install_requires=["cffi>=1.15.0"],
    setup_requires=["cffi>=1.15.0", "setuptools_scm"],

    cffi_modules=["setup_ffi.py:ffibuilder"],
)
