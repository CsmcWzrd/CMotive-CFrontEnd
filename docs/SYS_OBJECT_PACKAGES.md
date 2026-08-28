# Sys Object Package Revalidation

This pass converts Sys::Filesystem, Sys::Net, Sys::Thread, Sys::String, and Sys::Wide from function-first package surfaces to class/object-first package APIs. The older functional wrappers remain for source compatibility, but new CMotive code should prefer the classes below.

## Sys::Filesystem

- `Path` stores a path and exposes `Set`, `Get`, `Exists`, `IsFile`, `IsDirectory`, `MakeDirectory`, `Remove`, `RenameTo`, and `Size`.
- `Filesystem` exposes object methods for path operations without storing a path.

## Sys::Net

- `Socket` owns a native descriptor and exposes `OpenTcpIPv4`, `OpenTcpIPv6`, `OpenUdpIPv4`, `OpenUdpIPv6`, `OpenRawIPv4`, `OpenRawIPv6`, `OpenIcmpIPv4`, `OpenIcmpIPv6`, `BindIPv4`, `ConnectIPv4`, `Listen`, `Accept`, `Send`, `Recv`, `Close`, `Fd`, and `IsOpen`.
- `Net` exposes factory/close helpers.

## Sys::Thread

- `Thread` owns a native thread handle.
- `Threading` provides object methods for native-thread utilities.
- `MicroSleep` and `NanoSleep` are available and lower to `usleep`/`nanosleep` on POSIX and millisecond-rounded `Sleep` on Windows.

## Sys::String

- `CString` wraps `Char*` string state with length, compare, find, trim, case, conversion, and duplicate helpers.
- `Memory` wraps memory functions.
- `Character` wraps character classification/case helpers.
- `StringParser` owns the `str_parse` table handle and exposes `Parse`, `Rows`, `Cols`, `At`, and `Clear`.

## Sys::Wide

- `Wide16String` wraps `Char16*` helpers.
- `Wide32String` wraps `Char32*` helpers.

Functional wrappers are retained only as compatibility shims.
