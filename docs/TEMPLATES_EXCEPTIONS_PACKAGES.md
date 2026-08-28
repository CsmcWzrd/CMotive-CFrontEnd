# Templates, Exceptions, and Packages

This document records the implemented bootstrap compiler behavior for the three areas promoted from scaffold status.

## Template instantiation

`Template` declarations now retain their class or function body in the AST. During code generation, the compiler scans declarations and expressions for concrete uses such as:

```cmotive
Box<I32>
Identity<I32>(value)
```

For each concrete use, codegen creates a specialized class or function and substitutes template `Type` parameters with the concrete arguments. Native C symbols use a deterministic mangling form:

```text
TemplateName__Arg0__Arg1
```

Implemented now:

- concrete class template instantiation
- concrete function template instantiation
- template types in fields, locals, parameters, returns, and `New Template<T>()`
- recursive discovery of template uses introduced by generated specializations

Not yet implemented:

- partial specialization
- template constraints/concepts
- non-type template parameters
- destructor-finalization during exception unwinding through template-generated objects

## Exception unwinding

`Try`, `Catch`, `Catchall`, and `Throw` now lower to generated C with `setjmp`/`longjmp`. Each `Try` block creates a stack-local exception frame and pushes it onto the active exception stack. `Throw` records the message and unwinds to the nearest active frame. Unhandled exceptions print a message to `stderr` and exit with code `70`.

Implemented now:

- caught exceptions resume in `Catch`/`Catchall`
- return statements inside protected blocks pop active exception frames before returning
- uncaught exception diagnostics and stable exit code

Not yet implemented:

- typed exception class matching
- automatic destructor/finally execution during unwinding
- cross-translation-unit exception ABI verification

## Package loading

`Plugin` now performs real package/header/source resolution in the preprocessor. It materializes the package into the current translation unit before lexing/parsing.

Resolution examples:

```cmotive
Plugin Sys::Stdio
Plugin PackageUtil
```

Search candidates include:

```text
Sys/Stdio.HMOT
Sys/Stdio.HMTV
Sys/Stdio.CMOT
Sys/Stdio.CMTV
Sys/Stdio/package.HMOT
PackageUtil.HMOT
PackageUtil.CMOT
```

Search roots are the current file's directory, the CMotive `lib` directory, and any `-I` include paths.
