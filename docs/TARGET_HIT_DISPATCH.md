# Target and Hit Dispatch

This iteration implements CMotive `Target`/`Hit` dispatch in the bootstrap compiler.

## Syntax

A `Target` statement has four colon-delimited fields:

```cmotive
Target SenderClassName:Object:data1, data2, "data3":IdNumber;
```

The sender and object fields are optional but their colon positions remain meaningful:

```cmotive
Target ::1,2:90000;        // no sender and no object
Target :obj1:1,2:1234;     // no sender, object obj1
Target Sender::8:7000;     // sender-qualified, no object
Target Sender:obj1:5,6:8;  // sender-qualified object dispatch
```

A `Hit` prefix registers the following function or method as the handler for a sender/id route:

```cmotive
Hit :90000
Void
Print
x : I32
y : I32
()
{
    cout.expect("%d\n").write(x + y);
}
```

For methods, the `Target` object field is used as the receiver.  If the object is a stack object, the generated call passes `&object`; if it is a pointer, the generated call passes it directly.

## Lowering

`Hit` is compile-time metadata.  `Target` is lowered to a direct static dispatch call once the matching `(sender, id)` handler is known:

```c
StartPackage__Print(1, 2);
StartPackage__TargetReceiver__Add(&obj1, 3, 4);
```

If no handler is found, codegen emits a runtime diagnostic path:

```c
CMotive_UnresolvedTarget("Sender", 7000ULL);
```

This is intentionally not a Qt-style signal/slot implementation.  It is a CMotive-specific dispatch construct with deterministic static lowering and a guarded unresolved-route failure path.

## Coverage

Conformance tests exercise:

- `Target ::args:id` to `Hit :id` free functions.
- `Target :object:args:id` to object methods.
- `Target Sender::args:id` to sender-qualified free functions.
- `Target Sender:object:args:id` to sender-qualified methods.
