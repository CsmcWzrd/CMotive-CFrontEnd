# CMotive merged frontend/examples verification

Version: 0.2.5-cfrontend-merged

## Package merge

- Merged the former language examples package into the main CMotive source package.
- Included the provided CMotive v1 language definition under `docs/CMotive-v1-LanguageDefinition.md`.
- The 158 runnable language examples are now in `examples/`.
- Example helper headers and packages are in `examples/headers/` and `examples/packages/`.
- Added one-shot example makefiles:
  - `makefile.examples.linux`
  - `makefile.examples.mac`
  - `makefile.examples.windows`
- Added scripts:
  - `scripts/run_examples.sh`
  - `scripts/validate_language_files.sh`

## Source normalization

- Removed old bootstrap-style lowercase declarations from active `.CMOT`, `.CMTV`, `.HMOT`, and `.HMTV` files.
- Confirmed there are no `var Name` style declarations in active source/example/test/library files.
- Converted older conformance files from lowercase `func`, `class`, and `return` style to formal CMotive declaration syntax.

## Frontend review fixes

The native C frontend was reviewed and patched for the merged examples and whole-tree language-file validation. Fixes include:

- `Hit` declarations are parsed before `Name : Type` global declarations, so `Hit :90000` is no longer misread as a global variable declaration.
- Builtin `Mutex` runtime object shape now matches `Sys::Locks.HMOT` by using a `handle` member.
- Builtin `OStream` runtime object shape now matches `Sys::IO.HMOT` by exposing `fmt` and `handle` members.
- Template placeholder object types such as `Box<I32>` now compile cleanly in the C frontend scaffold path while concrete template ABI work matures.
- `New Template<T>()` fallback lowering emits `CMotive_New(1)` instead of raw generated C tokens.
- Class `__new` helpers return `void *`, avoiding C incompatible-pointer diagnostics when assigning a derived allocation to a base pointer.
- Existing previous fixes were retained: parser helper `accept` was renamed to `parser_accept`, Windows `small` collision was removed, Package/Plugin newline parsing was fixed, `Do/While`, `Switch/Case/Default`, `Elif`, scoped blocks, `Fptr`, nested classes, bit-member syntax, out-of-class `$Class` methods, `Blend`, `Enum`, template skipping, STL object aliases, and Sys object wrappers were preserved.

## Verification performed in Linux sandbox

Commands run:

```sh
make -f Makefile.linux clean all test
sh scripts/run_tests.sh build/bin '' --full
make -f makefile.examples.linux examples
```

Results:

```text
CMotive tests: PASS
CMotive examples: PASS (158 examples)
```

Whole-tree language-file validation was run in chunks to stay within the sandbox command timeout:

- 226 active `.CMOT`, `.CMTV`, `.HMOT`, and `.HMTV` files discovered, excluding `build`, `dist`, and `legacy`.
- 226 files successfully preprocessed with `cmotivepp`.
- 226 files successfully compile-checked with `cmotive -c`.
- 158 examples preprocessed.
- 158 examples compiled to runnable binaries.
- 158 example binaries executed with zero non-empty stderr logs.

Notes:

- Linux execution was performed in this sandbox.
- macOS and Windows example makefiles were created to mirror the Linux example workflow, but VS2022/macOS execution was not available in this environment.
