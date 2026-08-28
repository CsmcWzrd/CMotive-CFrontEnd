# CMotive Standard Library

The standard library package surface is stored under `lib/Sys` and uses CMotive
header syntax (`.HMOT`). Package files are real compiler inputs: `Plugin Sys::X`
loads the matching `lib/Sys/X.HMOT`, gives declarations their package identity,
and codegen lowers calls to package-qualified native symbols.

## Package surfaces

- `Sys::Stdio`
  - `print`, `println`, `puts`, `putchar`, `flush`
  - fluent `cout.expect(...).write(...)` / `cin.expect(...).read(...)` lowering
- `Sys::File`
  - `File` class with `open`, `close`, `read`, `write`, `seek`, `tell`, `eof`
  - free function wrappers for `open`, `close`, `read`, `write`, `flush`, `remove`, `rename`
- `Sys::Filesystem`
  - `exists`, `isFile`, `isDirectory`, `mkdir`, `remove`, `rename`, `size`, `currentPath`
- `Sys::Logging`
  - `setLevel`, `trace`, `debug`, `info`, `warn`, `error`, `fatal`
- `Sys::Thread`
  - `Thread` class plus `current`, `sleepMs`, `yield`, and a `Tstore` example surface
- `Sys::Locks`
  - `Mutex`, `RecursiveMutex`, `RwLock`, `SpinLock`, `ConditionVariable`, `Semaphore`
  - free function wrappers for mutex/rwlock lifecycle and lock/unlock flows
- `Sys::Net`
  - reserved network namespace with TCP/UDP/raw socket package surface
- `Sys::Exception`
  - `Exception` class, `throwText`, `throwCode`, `lastCode`
- `Sys::STL`
  - initial `Vector<T>` template package surface
- `Sys::Math`
  - broad Linux/libm-style surface: trig, hyperbolic, exponent/log, power/root,
    rounding, classification, float and long-double variants, and numeric helpers
- `Sys::String`
  - byte string, memory, conversion, case, trim, and parse helpers
  - `str_parse(input, recordDelims, fieldDelims, escape)` returns an opaque
    List/List or Vector/Vector-compatible parse table handle with `rows`, `cols`,
    `at`, and `free` accessors
- `Sys::Wide`
  - `Char16*` and `Char32*` length, compare, copy, concat, find helpers

## Runtime backing

The bootstrap compiler embeds portable C runtime helpers into generated C so
single-file examples link without separately building a runtime library. The same
helper surface is mirrored in `lib/Sys/runtime.c` for toolchains that prefer to
compile and link a standard runtime object explicitly.

The linker driver now adds `-pthread` and `-lm` on non-Windows native toolchains
because expanded `Sys::Locks` and `Sys::Math` helpers require pthread and libm
symbols on typical Linux systems.

## Remaining production work

This archive provides a broad runtime-backed API surface and conformance coverage.
The following are still intentionally not complete full-platform implementations:
real network socket operations beyond the reserved API surface, full userspace
scheduler integration, and STL container algorithms beyond the initial template
package shape.


## STL/Algorithms/IO/Net/Dynamic Struct implementation pass

See `docs/SYS_STL_ALGORITHMS_NET_IO_DYNAMIC.md` and `docs/FULL_TEST_SUITE_PLAN.md` for the expanded standard-library, native sockets, native threads, formatted IO, and Dynamic Struct update.


## Standard library object model audit
See `docs/STDLIB_OBJECT_MODEL_AUDIT.md`. The preferred `Sys::STL`, `Sys::IO`, and `Sys::Algorithms` surfaces are now class/object-method APIs; legacy functional helper lowering is retained only for compatibility.


## Sys object package update

`Sys::Filesystem`, `Sys::Net`, `Sys::Thread`, `Sys::String`, and `Sys::Wide` now expose class/object-first APIs. Compatibility functional wrappers remain, and `Sys::Thread` now includes `MicroSleep` and `NanoSleep`. See `SYS_OBJECT_PACKAGES.md`.
