# CMotive Programming Language Source Archive

CMotive is a production-oriented native language source tree. This archive has been updated so the active frontend and toolchain are implemented in C instead of Python for Linux, Windows, and macOS > 15.

The default command-line tools are now native C binaries:

- `cmotive`
- `cmotive++`
- `cmotivepp`
- `CMotiveSymsToDebugFile`

All four tools are built from `src/native/cmotivetool.c`. The executable name selects compiler, preprocessor, or symbol-tool behavior at runtime.

## Extensions

- Source: `.CMOT`, `.CMTV`
- Header: `.HMOT`, `.HMTV`

## Build

Linux:

```sh
make -f Makefile.linux all
make -f Makefile.linux test
make -f Makefile.linux full-test
./build/bin/cmotive -c examples/hello.CMOT -o build/hello.o
./build/bin/cmotive examples/hello.CMOT -o build/hello
```

macOS > 15:

```sh
make -f Makefile.mac all
make -f Makefile.mac test
```

Windows POSIX/MinGW/clang shell:

```sh
make -f Makefile.windows all
make -f Makefile.windows test
```

Use `CC=clang`, `CC=gcc`, or another C compiler to override the tool used to build the CMotive frontend. Use `CMOTIVE_CC` to override the downstream C compiler invoked by `cmotive` when it lowers CMotive source to generated C and links native artifacts.

No Python is used by the active build, test, package, compiler, preprocessor, or debug-symbol tool path.

## C frontend migration status

The active Python bootstrap frontend was moved out of the execution path and retained only under `legacy/python-bootstrap/` for reference and source-history comparison. The Makefiles no longer copy or execute Python scripts.

The native C frontend includes:

- CMotive preprocessing for `Plugin`, `Include`, `Replace`, and `Plugswitch`
- Lexer/parser and semantic checks
- CMotive-to-C lowering
- Native compile/link driver via the platform C compiler
- `cmotivepp` preprocessing mode
- `CMotiveSymsToDebugFile` debug-symbol sidecar generation
- Package-qualified symbols
- Class constructors/destructors, `New`/`Delete`, auto accessors, object-method lowering, standard library wrappers, target/hit dispatch, and conformance-test coverage for existing frontend behavior

See `docs/C_FRONTEND_MIGRATION.md` for the migration notes and verification commands.

## Status

This is a C-fronted compiler source scaffold with a native C compiler driver. The backend lowers CMotive to C and invokes the platform toolchain; repository scaffolds for native codegen, ARM64, x86_64, ABI/platform work, templates, exceptions, packages, plugins, and separate compilation remain in the tree for continued implementation.

See `docs/FEATURE_STATUS.md` for the full matrix.

## Current object-symbol ABI note

Class methods, constructors, destructors, and `New`/`Delete` helpers use package-qualified C symbols. If no `Package` declaration is active, the default package prefix is `StartPackage`, for example `StartPackage__ClassName__MethodName`.

## VS2022 native frontend project

Open `vs2022/CMotive.NativeFrontend.vcxproj` to build the C frontend with Visual Studio 2022. The post-build step copies the generated executable to `cmotivepp.exe`, `cmotive++.exe`, and `CMotiveSymsToDebugFile.exe` so the same native C binary provides all tools.

The older package scaffold project remains available for package-source references, but it is no longer the active frontend build path.


## VS2022 compatibility update

The native frontend has been adjusted for MSVC/Visual Studio 2022 C compilation:

- The parser helper previously named `accept` is now `parser_accept`, avoiding collision with Winsock's `accept()` declaration from Windows headers.
- The stack formatting buffer previously named `small` is now `stackbuf`, avoiding the Windows `small` macro/type collision.
- Windows builds alias `strtok_r` to MSVC `strtok_s`, preventing implicit-int pointer warnings under MSVC C mode.
- Temporary generated C files use `%TEMP%`/`%TMP%` on Windows instead of a hard-coded `/tmp` path.

## Complete feature pass update

The C frontend includes concrete paths for template-oriented lowering, exception cleanup frames, package loading, native sockets, STL helpers, auto `Get`/`Set`/`Getall`/`Setall` materialization, `Operation` overload lowering, `Tstore`/`ThreadStore`, package-scope `Global` declarations from any source location, and `Fptr` function-pointer typedef declarations. `Overridable` is the formal vtable keyword; pure virtual methods use `Overridable` and `()=0;` with no body.

## Debug symbols and optimization

`cmotive` supports `-g`, `-g2`, `-g3`, `-O1`, `-O2`, `-O3`, and `-Os`. Debug builds emit both native toolchain debug information and a human-readable CMotive symbol file via `CMotiveSymsToDebugFile`, named `<OutputName>_cmot_debugsymbols.syms`. See `docs/DEBUG_SYMBOLS_OPTIMIZATION.md`.

## Sys object standard library update

`Sys::Filesystem`, `Sys::Net`, `Sys::Thread`, `Sys::String`, and `Sys::Wide` expose class/object-first APIs, with compatibility wrappers retained. `Sys::Thread` includes `MicroSleep` and `NanoSleep`.
