# CMotive language examples verification report

Updated for the native C frontend pass.

Verified in this archive:

- `make clean`: PASS
- `make manifest CMOTIVE_ROOT=../cmotive-source`: PASS, 158 manifest examples present
- `make debug-symbols CMOTIVE_ROOT=../cmotive-source`: PASS using the native C-built `cmotive` binary

The active examples Makefile now uses `tools/example_build.sh` and `../cmotive-source/build/bin/cmotive` / `cmotivepp` by default. The earlier Python example helpers are retained only in `legacy/python-example-tools/` for reference and are not used by the active Makefile.

The examples include coverage for debug symbols, `Sys::IO`, STL containers, `Sys::Algorithms`, native sockets, native threading, Dynamic Struct `Expand`, package-scope `Global`, `Fptr`, `Overridable` pure virtual declarations, `ThreadStore`/`Tstore`, object-oriented `Sys::STL`, `Sys::Algorithms`, `Sys::IO`, `Sys::Filesystem`, `Sys::Net`, `Sys::Thread` MicroSleep/NanoSleep, `Sys::String`, and `Sys::Wide` APIs.
