# CMotive Sys::IO, STL, Algorithms, Net, Thread and Dynamic Struct update

This iteration renames the preferred formatted I/O package from `Sys::Stdio` to `Sys::IO`, while keeping `Sys::Stdio` as a compatibility wrapper. The generated runtime maps `cout.expect(format).write(...)` to `printf` and `cin.expect(format).read(...)` to `scanf`, so `expect` is now treated as a printf/scanf-style format string.

`Sys::STL` now exposes Vector, List, Dlist, Map, Dict, HashDict, MultiDict, MultiHashDict, BinaryTree, BinarySearchTree, BTree and BPlusTree package surfaces. The first concrete runtime implementation uses generic integer vector/map/tree handles plus package-level helper functions. Dict is treated as a synonym for Map. BTree and BPlusTree currently share the ordered tree runtime backend pending page-node balancing work.

`Sys::Algorithms` includes concrete sorting and searching helpers: quick/qsort-backed sort, heap/merge aliases, insertion, selection, bubble, shell, comb, gnome, radix/counting aliases, binary search, linear search, jump/exponential/interpolation search aliases, min/max, and comparison helpers.

`Sys::Thread` uses native system threads: pthreads on POSIX and CreateThread handles on Windows. Userspace scheduling is intentionally not added in this pass.

`Sys::Net` includes native TCP/UDP IPv4/IPv6 sockets, raw socket creation, ICMP socket creation, close/send/recv/bind/connect/listen/accept helpers. Raw/ICMP socket creation may require administrator/root privileges, so conformance tests only open TCP/UDP sockets by default.

`Dynamic Struct` is a compile-time grow-only struct form. A declaration creates the initial C struct, and any `TypeName Expand { ... }` statement contributes additional fields before C emission. Duplicate field names are ignored after the first declaration.
