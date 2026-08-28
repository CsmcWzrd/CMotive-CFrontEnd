# CMotive Requirements Verification

This report records the implementation pass against the supplied `CMotive.md`
language requirements.

## Implemented in this iteration

| Requirement area | Status | Updated implementation |
|---|---:|---|
| File extensions `.CMOT`, `.CMTV`, `.HMOT`, `.HMTV`, any case | Implemented | `tools/cmotive.py` accepts source and header extensions in upper/lower case. |
| Capitalized CMotive keywords | Implemented | `src/cmotive/lexer.py` contains the formal CMotive keyword table, compatibility aliases, and the new integer type synonyms `Int16`/`Int32`/`Int` and `Uint16`/`Uint32`/`Uint`. |
| Formal line-oriented function declarations | Implemented | `src/cmotive/parser.py` parses `ReturnType`, function name, parameter lines, `()`, and body. |
| Old bootstrap `func`/`var` compatibility | Preserved | Parser still accepts legacy tests/examples. |
| Classes, visibility blocks, constructors, destructors | Implemented in bootstrap compiler | Parser accepts `Class`, visibility blocks, constructors/destructors, nested classes, and methods; codegen lowers classes to C structs and methods to package-qualified mangled `Package__Class__Method(this, ...)` functions, using `StartPackage` when no package is declared. |
| Single inheritance | Implemented in bootstrap compiler | Semantic analysis enforces one base, validates that the base exists, detects inheritance cycles, and codegen embeds the base struct as the first field for ABI-compatible upcast layout. |
| Bit member specification | Implemented scaffold | Parser records bit fields; codegen lowers them to C bit-field structs. |
| `Blend`/`Enum` | Parse scaffold | Parser preserves declarations as scaffold metadata without breaking compilation. |
| `Template` and `Type` | Implemented in bootstrap compiler | Parser records template class/function bodies; codegen instantiates concrete class and function templates when `Name<T...>` is used. |
| `Try`/`Catch`/`Catchall`/`Throw` | Implemented in bootstrap compiler | Codegen emits stack-local exception frames with `setjmp`/`longjmp`; stack class objects created in protected `Try` scopes register destructor cleanup frames that run during unwinding; uncaught exceptions exit with code 70. |
| Control flow | Implemented | `If`/`Elif`/`Else`, `While`, `Do`/`While`, `For`, raw `Switch`/`Case`/`Default`, `Break`, and `Continue` lower to C. |
| `New`/`Delete` | Implemented in bootstrap compiler/runtime | `New Class(...)` lowers to generated package-qualified typed `Package__Class__new__<Type...>` helpers using `CMotive_New` followed by type-resolved constructor dispatch; `Delete obj` lowers to `Package__Class__delete`, destructor dispatch, and `CMotive_Delete`. |
| Standard operators including `>>>` and `<<<` | Partial implemented | C/C++ operators pass through; rotate shifts lower to helper functions for simple identifiers. |
| `Contains { ... }` strings | Implemented | Lexer folds `Contains` blocks into string tokens. |
| `Package`/`Plugin` | Implemented in preprocessor | `Plugin` resolves real package/header/source files from the source directory, `lib`, and `-I` include paths, materializes them before parsing, and restores the importing file package context afterwards. |
| `Plugswitch`/`Plugcase`/`Plugdefault`/`Plugend` | Implemented scaffold | `src/cmotive/preprocessor.py` selects OS, processor, endian and defined-expression cases. |
| `Replace` | Implemented | Preprocessor handles `Replace` as a CMotive macro definition. |
| Native binary output | Implemented | `tools/cmotive.py` compiles to C, emits `.o`/`.obj` with `-c`, and links executables with platform linker path. |
| Processor targets ARM, ARM64, x86, x86_64 | Implemented CLI scaffold | `--target-arch` accepts `arm`, `arm64`/`aarch64`, `x86`, `x64`/`x86_64`; x86_64 has a default native toolchain path using canonical target markers and `-m64`/Darwin `-arch x86_64` flags. |
| macOS ARM64 linker flag | Implemented | Darwin builds pass `-arch <target>` for compile and link. |
| Strip-compatible symbols | Implemented scaffold | Native object/executable generation uses normal platform toolchain/debug symbol format. |
| Sys::Stdio | Implemented scaffold | Fluent `cout.expect(...).write(...)` and `cin.expect(...).read(...)` lower to C stdio. |
| Sys::File, Filesystem, Logging, Thread, Net, STL, Exception | Expanded package surfaces | Headers under `lib/Sys` now include broader runtime-backed APIs; `Sys::STL` template headers still exercise real template instantiation and `Sys::Net` keeps deterministic reserved placeholder behavior. |
| Sys::Locks, Math, String, Wide | Implemented package surfaces | Added `lib/Sys/Locks.HMOT`, `Math.HMOT`, `String.HMOT`, and `Wide.HMOT`; generated C embeds portable helper implementations and `lib/Sys/runtime.c` mirrors them for explicit runtime builds. |
| `Target`/`Hit` dispatch | Implemented in bootstrap compiler | Parser records `Target` statements and `Hit` function/method prefixes; codegen registers handlers by `(sender, id)` and lowers targets to direct static calls or guarded unresolved-route diagnostics. |

## Known limitations after this iteration

- The native C++ compiler sources are still implementation scaffolds; the Python bootstrap
  compiler path is the executable implementation.
- Userspace threads, concrete socket operations, and full STL container algorithms remain package-stable scaffolds; Sys::Locks and most Sys::Math/String/Wide helpers now have runtime-backed behavior.
- Template instantiation is implemented for concrete type-parameter class/function templates in the bootstrap compiler; advanced constraints/partial specialization are not yet implemented.
- Exception unwinding is implemented with `setjmp`/`longjmp` in generated C; destructor cleanup is now implemented for stack objects created in generated `Try` scopes. Full automatic lifetime finalization for every block remains future work.
- Multiple inheritance is rejected for this CMotive single-inheritance implementation; one concrete base is validated and lowered.
- Operator overloading with `Operation`, auto `Get`/`Set`/`Getall`/`Setall`, and full
  dynamic struct materialization are not fully implemented.
- RTTI remains intentionally out of scope for this version.

## Verification performed

`make -f Makefile.linux test` passed after the implementation pass.  The tests now
cover legacy bootstrap syntax, formal CMotive line-oriented syntax, class/header parsing, concrete class struct lowering, package-qualified method mangling, constructor/destructor chaining, type-based constructor overload resolution, virtual dispatch through vtable slots, destructor cleanup during exception unwinding, `New`/`Delete` runtime dispatch, keyword/type synonyms, x86_64 native target output, invalid-base diagnostics, control flow, preprocessor selection, concrete template instantiation, caught exception unwinding, real Plugin package loading, object generation, executable generation, emitted C, Target/Hit direct/object/sender dispatch, expanded Sys package compilation/execution, `str_parse`, `Sys::Locks`, `Sys::Math`, `Sys::String`, `Sys::Wide`, and preprocessing.


## STL/Algorithms/IO/Net/Dynamic Struct implementation pass

See `docs/SYS_STL_ALGORITHMS_NET_IO_DYNAMIC.md` and `docs/FULL_TEST_SUITE_PLAN.md` for the expanded standard-library, native sockets, native threads, formatted IO, and Dynamic Struct update.


## Complete feature pass update

The bootstrap compiler now includes concrete paths for full-template instantiation, exception unwinding with destructor cleanup frames, real package loading, native sockets, STL helpers, auto Get/Set/Getall/Setall materialization, `Operation` overload lowering, `Tstore`/`ThreadStore`, package-scope `Global` declarations from any source location, and `Fptr` function-pointer typedef declarations. `Overridable` is the formal vtable keyword; pure virtual methods use `Overridable` and `()=0;` with no body.


## Debug symbols and optimization update

- Added `CMotiveSymsToDebugFile` to emit `<OutputName>_cmot_debugsymbols.syms` with 64-bit native function offsets, mangled symbol names, CMotive prototypes, C prototypes, package/class/source fields, and fallback native-symbol rows.
- Added compiler options `-g`, `-g2`, `-g3`, `-O1`, `-O2`, `-O3`, and `-Os`. Debug options generate CMotive metadata and `.syms` files; optimization options are forwarded to the native toolchain.


## Standard library object model audit
See `docs/STDLIB_OBJECT_MODEL_AUDIT.md`. The preferred `Sys::STL`, `Sys::IO`, and `Sys::Algorithms` surfaces are now class/object-method APIs; legacy functional helper lowering is retained only for compatibility.


## Sys object package update

`Sys::Filesystem`, `Sys::Net`, `Sys::Thread`, `Sys::String`, and `Sys::Wide` now expose class/object-first APIs. Compatibility functional wrappers remain, and `Sys::Thread` now includes `MicroSleep` and `NanoSleep`. See `SYS_OBJECT_PACKAGES.md`.
