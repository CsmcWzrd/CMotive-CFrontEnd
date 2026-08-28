# CMotive Debug Symbols and Optimization

This release adds a CMotive debug-symbol export path and GCC-like debug and optimization command-line options.

## Compiler options

`cmotive` now accepts:

- `-g`  : generate level-1 native debug information and CMotive symbol metadata.
- `-g2` : generate level-2 native debug information and CMotive symbol metadata.
- `-g3` : generate level-3 native debug information and CMotive symbol metadata.
- `-O1` : optimize generated native code.
- `-O2` : stronger optimization.
- `-O3` : speed-oriented optimization.
- `-Os` : size-oriented optimization.

For GCC/Clang-like toolchains these options are forwarded as `-g`, `-g2`, `-g3`, `-O1`, `-O2`, `-O3`, or `-Os`. For MSVC-like `cl`, the compiler maps debug generation to `/Z7` or `/Zi`, and maps optimization to `/O1` or `/O2` where possible.

## Generated files

When an executable, object, or library is built with `-g`, `-g2`, or `-g3`, the compiler writes:

```text
<OutputName>.cmotive.debug.json
<OutputName>_cmot_debugsymbols.syms
```

The JSON file is compiler metadata. The `.syms` file is intended for humans and build/release systems.

## `CMotiveSymsToDebugFile`

`CMotiveSymsToDebugFile` can also be run manually:

```sh
CMotiveSymsToDebugFile build/app --metadata build/app.cmotive.debug.json
```

Default output:

```text
build/app_cmot_debugsymbols.syms
```

Each non-comment row contains:

```text
offset64 | symbol | kind | package | class | source | prototype | c_prototype
```

The `offset64` column is read from the native executable/library/object symbol table using `nm`, `llvm-nm`, or `objdump` when available. If the native symbol cannot be resolved, the offset is emitted as `0x????????????????` while still preserving the CMotive prototype metadata.

## Example

```sh
./build/bin/cmotive -g3 -O2 tests/conformance/cmotive_debug_symbols.CMOT -o build/debug_symbols
cat build/debug_symbols_cmot_debugsymbols.syms
```

A typical row looks like:

```text
0x00000000000011A0 | StartPackage__DebugThing__Add | method | StartPackage | DebugThing | tests/conformance/cmotive_debug_symbols.CMOT | I32 StartPackage::DebugThing::Add(y: I32) | int32_t StartPackage__DebugThing__Add(DebugThing *this, int32_t y);
```
