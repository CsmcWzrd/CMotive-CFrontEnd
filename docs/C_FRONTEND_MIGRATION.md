# CMotive C Frontend Migration

## Goal

The active CMotive frontend/toolchain has been moved from Python bootstrap scripts to a native C implementation for Linux, Windows, and macOS > 15.

## Active tool source

`src/native/cmotivetool.c` builds the complete command-line tool family:

- `cmotive`
- `cmotive++`
- `cmotivepp`
- `CMotiveSymsToDebugFile`

The tool dispatches behavior from the executable name. This keeps the installed command surface compatible while avoiding Python in the active frontend path.

## Python status

Python code is retained only under `legacy/python-bootstrap/` for reference. It is not used by the active Makefiles, test target, package target, compiler, preprocessor, or debug-symbol tool.

## Build commands

Linux:

```sh
make -f Makefile.linux clean all test
make -f Makefile.linux full-test
```

macOS > 15:

```sh
make -f Makefile.mac clean all test
```

Windows POSIX/MinGW/clang shell:

```sh
make -f Makefile.windows clean all test
```

## Toolchain overrides

- `CC` controls the C compiler used to build the CMotive frontend itself.
- `CMOTIVE_CC` controls the C compiler that `cmotive` invokes for generated C output.

Examples:

```sh
make -f Makefile.linux CC=clang all test
CMOTIVE_CC=clang ./build/bin/cmotive examples/hello.CMOT -o build/hello
```

## Verification coverage

The shell-based test runner `scripts/run_tests.sh` verifies:

- tool version execution
- object generation via `-c`
- native executable generation via `-o`
- debug symbol sidecar generation
- `Sys::IO` compatibility renames
- STL containers and algorithms
- native socket and thread wrappers
- dynamic struct expansion
- auto accessors
- operation overload lowering
- `Tstore`/`ThreadStore`
- `Global` declarations
- `Fptr` function pointers
- overridable/pure virtual scaffolding
- object-first `Sys::*` APIs
- CMotive preprocessor behavior
- constructor overloads
- target/hit dispatch
- invalid-base semantic failure

## Packaging

`make -f Makefile.linux package` uses the shell script `scripts/package_release.sh` and creates `dist/cmotive-<version>-cfrontend.tar.gz` and, when `zip` is available, `dist/cmotive-<version>-cfrontend.zip`.
