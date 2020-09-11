Pack
====

Pack is a simple serialisation and deserialisation library for C.

Dependencies
------------

- A DJB redo compatible redo implementation
- A relatively recent copy of bash
- gcc with C11 support (To use clang, edit `link` and `default.o.do`)

Compilation
-----------

Optionally run `./configure release` to enable good additional default flags.
Alternatively, copy the files in `args/release/` to the project root and add
additional linker or compiler arguments necessary to build on your machine into
link.args and compile.args respectively. Please report flags necessary to build
on your platform so that they can be documented and potentially even
auto-detected.

Run `redo` to build the code and tests.

Testing
-------

Run `test` to run all the tests.

Development
-----------

Development args for the compiler and linker are stored in `args/devel/` and can
be copied from there manually or symlinked with `./configure devel`.

Contributing
------------

To contribute, please send a patch to me at <tk+pack-patches@the-tk.com>.
