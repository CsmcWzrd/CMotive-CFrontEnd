# Full CMotive test-suite plan

1. Lexer/parser conformance for every keyword, synonym, extension case, package form, Target/Hit route, Dynamic Struct declaration, and Dynamic Struct expansion.
2. Codegen golden tests for classes, inheritance, constructors/destructors, virtual tables, packages, templates, exceptions, Target/Hit, Dynamic Structs, STL calls, IO, Net and native threading.
3. Runtime smoke tests for safe deterministic IO, String, Wide, Math, STL, Algorithms, Locks, Thread and filesystem/file helpers.
4. Native socket tests split into default safe tests and privileged opt-in tests. Default tests open/close TCP/UDP IPv4/IPv6 sockets; raw/ICMP tests run only when explicitly enabled.
5. Cross-platform matrix for Linux x86_64, Linux ARM64, macOS x86_64/ARM64, and Windows x64/VS2022.
6. ABI checks for strip-compatible objects, package-qualified symbols, virtual table layout, constructor/destructor signatures, and linker output.
7. Negative/fuzz tests for malformed templates, malformed Dynamic Struct expansions, duplicate fields, invalid package names, invalid constructor overloads, and unresolved Target/Hit dispatch.
8. Lang-Examples gate: build object-only, link, run, and verify exit code/stderr for every manifest entry.
