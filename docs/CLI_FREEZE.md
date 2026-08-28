# CLI Freeze

The CMotive 0.2 RC CLI keeps the original tool names and build expectations:

- `cmotivepp` — CMotive preprocessor.
- `cmotive` — compiler driver.
- `cmotive++` — alias/same compiler entry point.

Supported options:

- `--version`
- `-c`
- `-o <path>`
- `-I <dir>`
- `-L <dir>`
- `-l <lib>`
- `--target-arch arm|arm64|aarch64|x86|x64|x86_64|amd64|i386|i686`
- `--emit-c`
- `--keep-c`
- `--print-linker`

`-c` emits native `.o`/`.obj` files through the platform compiler.  Without `-c`,
the driver invokes `CMOTIVE_LD` if set, otherwise the platform compiler driver, to
produce a native executable at `-o` or `a.out`.


## Debug symbols and optimization update

- Added `CMotiveSymsToDebugFile` to emit `<OutputName>_cmot_debugsymbols.syms` with 64-bit native function offsets, mangled symbol names, CMotive prototypes, C prototypes, package/class/source fields, and fallback native-symbol rows.
- Added compiler options `-g`, `-g2`, `-g3`, `-O1`, `-O2`, `-O3`, and `-Os`. Debug options generate CMotive metadata and `.syms` files; optimization options are forwarded to the native toolchain.
