# Classes, Single Inheritance, Constructors, Destructors, Virtual Dispatch, New/Delete

This iteration moves the CMotive bootstrap compiler beyond placeholders for the
core object model.

## Implemented behavior

- `Class` declarations lower to concrete C `struct` definitions.
- A root class receives a `CMotiveObject __cmotive_object` header.
- A derived class embeds its validated base class as the first field named
  `__base`, giving the generated C layout an ABI-compatible first-member base
  layout.
- Methods lower to mangled C functions in the form:

  ```c
  ReturnType PackageName__ClassName__MethodName(ClassName *this, ...);
  ```

- If no `Package` declaration has appeared for a declaration, codegen uses
  `StartPackage` as the default package prefix.
- `Overridable` methods and overrides of inherited overridable methods now
  populate generated `CMotiveVTable` slot tables.
- Virtual calls made through `obj.Method()` or `ptr->Method()` lower through the
  receiver object's runtime vtable slot when the static class declares that
  method as virtual/overridable. Non-virtual methods still lower to direct
  package-qualified calls.
- Constructors named as the class lower to `PackageName__ClassName__ctor` for zero
  parameters and `PackageName__ClassName__ctor__<Type...>` for typed overloads.
- Constructor overload resolution now uses inferred argument types, not only
  argument count. `Int32` resolves to the `I32` overload and `Int` resolves to
  the `I64` overload.
- Destructors declared as `~ClassName` lower to `PackageName__ClassName__dtor`.
- Derived constructors call the zero-argument base constructor before field
  initializers and the derived body, then install the derived vtable.
- Derived destructors run the derived body and then call the base destructor.
- `New ClassName(...)` lowers to a generated
  `PackageName__ClassName__new__<Type...>(...)` helper backed by `CMotive_New`.
- `Delete objectPointer` lowers to `PackageName__ClassName__delete(objectPointer)`,
  which dispatches `PackageName__ClassName__dtor` and then calls `CMotive_Delete`.
- Stack objects created inside protected `Try` scopes register destructor cleanup
  frames. On `Throw`, generated `setjmp`/`longjmp` exception handling runs those
  destructors before entering `Catch`/`Catchall`.
- `Plugin`-loaded packages restore the importing translation unit's package
  context after the imported file is materialized, so imported `Package`
  declarations do not accidentally retag following user code.

## Validation

Semantic analysis rejects:

- an undefined base class;
- multiple base classes, because this implementation targets CMotive's
  single-inheritance requirement;
- inheritance cycles;
- duplicate methods with the same simple signature;
- constructor/destructor declarations whose names do not match the enclosing
  class.

## Remaining limits

- Virtual dispatch is implemented for method-name slots; overloaded virtual
  method slots are not yet split by full method type signature.
- Destructor cleanup is implemented for stack objects constructed in generated
  `Try` scopes; full C++-style automatic lifetime finalization for every block
  and every early exit is still not complete.
- Constructor overload inference covers declared variables, casts, literals, and
  common numeric compatibility; a full expression type checker is still future
  work.
