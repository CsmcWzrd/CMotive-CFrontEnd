# Expanded Sys Packages Update

This iteration turns the previous Sys package scaffolds into broader runtime-backed
surfaces and adds new packages requested by the CMotive language requirements.

## Added packages

- `Sys::Locks` — mutex, recursive mutex, read/write lock, spin-lock facade,
  condition variable, semaphore, plus free wrapper functions.
- `Sys::Math` — common Linux `libm`-style double/float/long-double math helpers.
- `Sys::String` — string, memory, conversion, case, trim, and `str_parse` helpers.
- `Sys::Wide` — `Char16*` and `Char32*` wide-string helpers.

## Expanded packages

- `Sys::Stdio` gained additional print/flush helpers.
- `Sys::File` gained open/read/write/seek/tell/eof/flush/remove/rename surfaces.
- `Sys::Filesystem` gained file/directory queries, rename, size, current path.
- `Sys::Logging` gained trace/debug/info/warn/error/fatal and level filtering.
- `Sys::Thread` gained sleep/yield/current and a larger thread class surface.
- `Sys::Net` keeps the TCP/UDP/raw API reserved with deterministic placeholder behavior.
- `Sys::Exception` gained `Exception.what`, `throwCode`, and `lastCode`.

## `str_parse`

`str_parse` accepts two delimiter sets and one escape character:

```cmotive
parsed : Void* = str_parse("a,b;c,d", ";", ",", '\\');
rows : U64 = str_parse_rows(parsed);
cols : U64 = str_parse_cols(parsed, 0);
cell : Char* = str_parse_at(parsed, 1, 1);
str_parse_free(parsed);
```

The return value is an opaque parse-table handle intended to represent a
`List<List<Char*>>` or `Vector<Vector<Char*>>`-compatible structure while the
CMotive STL container ABI continues to mature.
