# setuptools supercedes distutils (mostly)
from setuptools import setup, Extension

pymongocrypt_module = Extension('pymongocrypt',
    sources = ['pymongocrypt.c'],
    include_dirs = ["./libmongocrypt/"],
    library_dirs = ["./libmongocrypt"],
    libraries = ["mongocrypt"],
    # for some reason, pip install installs libmongocrypt.dylib into the root of the virtual environment.
    extra_link_args = ["-rpath", "@loader_path", "-rpath", "@loader_path/libmongocrypt"])

# adding things to data_files just puts them in the egg.
setup (name = 'pymongocrypt',
        version = '0.1',
        description = 'A proof-of-concept',
        ext_modules = [pymongocrypt_module],
        data_files = ["./libmongocrypt/libmongocrypt.dylib"],
        packages = ["pymongocrypt"],
        package_dir = {"pymongocrypt": "."},
        package_data = {"pymongocrypt": ['./libmongocrypt/libmongocrypt.dylib']},
        zip_safe = False
        )
