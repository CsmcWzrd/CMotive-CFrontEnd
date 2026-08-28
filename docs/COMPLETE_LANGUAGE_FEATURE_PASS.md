# Complete Language Feature Pass

This iteration converts the remaining user-visible scaffolds into executable compiler/runtime paths for the current CMotive bootstrap compiler.

Implemented or expanded:

- Full-template instantiation for concrete class/function template uses handled by the bootstrap compiler.
- Exception unwinding through `Try`, `Catch`, `Catchall`, and `Throw`, including destructor cleanup frames for protected stack objects.
- Real `Plugin` package loading from current source folder, `lib`, and include paths for `.HMOT`, `.HMTV`, `.CMOT`, and `.CMTV` files.
- Native sockets in `Sys::Net`: TCP/UDP/raw/ICMP surfaces for IPv4 and IPv6 plus close/send/recv/bind/connect/listen/accept helpers.
- Expanded `Sys::STL` container helpers for Vector, List, Dlist, Dict/Map, HashDict, MultiDict, MultiHashDict, and tree-family containers.
- `Get`, `Set`, `Getall`, and `Setall` materialization for public non-`Block` class members.
- `Operation` operator-overload declarations and direct lowering for supported class binary operations.
- `Tstore` and `ThreadStore` storage-class support, lowered to C thread-local storage.
- `Overridable` is the formal CMotive keyword for vtable dispatch. Pure virtual declarations use `Overridable` and the only syntax difference is `()=0;` with no body.
- `Global` may prefix a variable declaration or appear in its type specifier; either form hoists the variable to package-global storage even when written inside a function or method.
- `Fptr` may prefix a function declaration to emit a function-pointer typedef with that function name.

New conformance tests:

- `cmotive_global_anywhere.CMOT`
- `cmotive_fptr_function_pointer.CMOT`
- `cmotive_overridable_pure_virtual.CMOT`

Verification:

```sh
python3 -m py_compile src/cmotive/*.py tools/cmotive.py tools/cmotivepp.py tests/run_tests.py
make -f Makefile.linux clean all test
python3 tests/run_tests.py --bin build/bin --full
```

Both release and full conformance suites passed in this iteration.
