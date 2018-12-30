Why doesn't `python setup.py develop` work?

Someone else trying to do what I did:
https://stackoverflow.com/questions/37316177/distribute-a-python-package-with-a-compiled-dynamic-shared-library

Finally, a good explanation of runtime linking on macos:
https://matthew-brett.github.io/docosx/mac_runtime_link.html

The gist is: rpath allows multiple paths to be checked when linking at runtime.
By default, relative paths are relative to to the executable.
@loader_path refers to the location of the library being linked against.

And how do you add to rpath with setuptools? Setting runtime_library_dirs seemed promising:
https://mail.python.org/pipermail/python-list/2003-October/238743.html
But looking at the compilation commands, it looked like it wasn't actually passing -rpath
```
# runtime_library_dirs = ["@loader_path"], This shows up in a resulting command as -L:
# cc -bundle -undefined dynamic_lookup -Wl,-F. build/temp.macosx-10.14-intel-2.7/pymongocrypt.o -L./libmongocrypt -L@loader_path -lmongocrypt -o build/lib.macosx-10.14-intel-2.7/pymongocrypt.so
# ld: warning: directory not found for option '-L@loader_path'
```

So instead, I just passed it manually through extra_linker_args:
`extra_link_args = ["-rpath", "@loader_path"]`

That *still* didn't work. But I noticed after unzipping the egg and loading the extension with `python -i` it *did* work in that directory. The culprit seemed to be that it wasn't able to load the dylib within the zipped egg. So I had to pass `zip_safe=False`.

(Note, this seems to contradict https://setuptools.readthedocs.io/en/latest/setuptools.html#setting-the-zip-safe-flag, which says the zip_safe check is "extremely conservative")... sure.

To look at something's dependant libraries: `otool -L something`
To check all of the rpath values, look at the end of `otool -l something` for LC_RPATH entries.
To modify an rpath, use `install_name_tool`.