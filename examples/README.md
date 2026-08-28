# CMotive Language Examples

This archive contains runnable CMotive examples plus header and package/VS2022 scaffolds. It has been updated to use the native C CMotive frontend by default.

Expected sibling layout after extraction:

```text
workspace/
  cmotive-source/
    build/bin/cmotive
    build/bin/cmotivepp
  cmotive-language-examples/
    Makefile
    examples/
    headers/
    packages/
    vs2022/
```

Build the C frontend first:

```sh
cd ../cmotive-source
make -f Makefile.linux all test
```

Then verify examples:

```sh
cd ../cmotive-language-examples
make check
```

Override paths when needed:

```sh
make check CMOTIVE_ROOT=/path/to/cmotive-source
make check CMOTIVE=/path/to/cmotive CMOTIVEPP=/path/to/cmotivepp
```

Useful targets:

```sh
make compile      # compile every manifest example into build/bin
make objects      # compile every manifest example with -c into build/obj
make run          # compile and execute every manifest example
make preprocess   # preprocess every manifest example into build/pp
make debug-symbols
make clean
```

The example automation is now `tools/example_build.sh`. The earlier Python helper scripts are retained only in `legacy/python-example-tools/` for reference and are not used by the active Makefile.

The examples are aligned with the formal CMotive requirements: capitalized keywords, `.CMOT/.CMTV/.HMOT/.HMTV` extensions, line-oriented functions, classes, inheritance, constructors/destructors, `New`/`Delete`, control flow, templates/blend/enum scaffolds, exception scaffolds, Package/Plugin syntax, standard-library package usage, and platform/ABI-oriented source shapes.

Manifest coverage includes examples for `Sys::IO` rename, STL containers, `Sys::Algorithms`, native sockets, native threading, Dynamic Struct `Expand`, package-scope `Global`, `Fptr` function pointers, `Overridable` pure virtual declarations, `ThreadStore`/`Tstore`, debug-symbol metadata, and object-oriented `Sys::Filesystem`, `Sys::Net`, `Sys::Thread`, `Sys::String`, and `Sys::Wide` APIs.
