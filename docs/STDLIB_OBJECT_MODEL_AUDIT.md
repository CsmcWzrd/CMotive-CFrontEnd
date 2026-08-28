# CMotive Standard Library Object Model Audit

This pass replaces the previously functional-only `Sys::*` package surfaces with class/object style CMotive APIs. `Sys::STL` now exposes object methods on `Vector`, `List`, `Dlist`, `Map`, `Dict`, `HashDict`, `MultiDict`, `MultiHashDict`, `BinaryTree`, `BinarySearchTree`, `BTree`, and `BPlusTree`. The method surface follows C++23 container categories: constructors/destructors, element access, capacity, modifiers, iterator endpoints, lookup, sorting/searching where applicable, and swap/clear/reserve operations.

Compatibility: existing package-level compiler lowering and runtime helpers remain available so older examples continue to compile. New examples and tests use object-method syntax.

`Sys::IO` now includes `OStream`, `IStream`, and `Formatter` classes while preserving `cout.expect(...).write(...)`, `cin.expect(...).read(...)`, and package-level wrappers.

`Sys::Algorithms` now exposes an `Algorithms` class with sorting, searching, min/max/compare, `IsSorted`, `Reverse`, `RotateLeft`, and `Unique` methods.

Implementation note: `BTree` and `BPlusTree` use the same ordered-tree runtime backend in this release; the public API shape is class/object based and test-covered, while page-node B/B+ balancing remains a future internal storage optimization.
