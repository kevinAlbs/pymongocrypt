# setuptools supercedes distutils (mostly)
from setuptools import setup, Extension

pymongocrypt_module = Extension('pymongocrypt',
    sources = ['pymongocrypt.c'],
    include_dirs = ["./libmongocrypt/"],
    library_dirs = ["./libmongocrypt"],
    libraries = ["mongocrypt"],
    extra_link_args = ["-rpath", "@loader_path"])

# adding things to data_files just puts them in the egg.
setup (name = 'pymongocrypt',
        version = '0.1',
        description = 'A proof-of-concept',
        ext_modules = [pymongocrypt_module],
        data_files = ["./libmongocrypt/libmongocrypt.dylib"],
        zip_safe = False,
        packages = ['wrapper']
        )
