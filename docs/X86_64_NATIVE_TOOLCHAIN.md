# x86_64 Native Toolchain Path

CMotive's bootstrap compiler supports the x86_64 processor target through the
same production-oriented native toolchain path used by the other current targets:
CMotive source is parsed, semantically checked, lowered to generated C, compiled
to a native object, and linked into a native executable.

## Implemented behavior

- Target aliases `x64`, `amd64`, and `x86_64` canonicalize to `x86_64`.
- Generated C includes a `target-arch: x86_64` provenance marker when requested.
- Linux/Unix builds pass `-m64` to the native compiler/linker front end.
- macOS builds pass `-arch x86_64` to the compiler/linker front end.
- Windows builds use the selected x64 Visual Studio/clang environment.
- Outputs remain normal platform-native object/executable files and retain
  strip-compatible debug/symbol metadata.

## Verification

The conformance suite emits an x86_64-marked C file and also verifies native
x86_64 executable generation on x86_64 Linux hosts.
