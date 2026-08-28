#!/bin/sh
set -eu
BIN_DIR="${1:-build/bin}"
EXEEXT="${2:-}"
FULL="${3:-}"
CMOTIVE="$BIN_DIR/cmotive$EXEEXT"
mkdir -p build
build_run() {
  src="$1"
  out="$2"
  "$CMOTIVE" "$src" -o "build/$out$EXEEXT"
  "build/$out$EXEEXT"
}
"$CMOTIVE" --version
"$CMOTIVE" -c tests/conformance/basic.CMOT -o build/basic.o
build_run tests/conformance/basic.CMOT basic
dbg="build/debug_symbols$EXEEXT"
"$CMOTIVE" -g3 -O2 tests/conformance/cmotive_debug_symbols.CMOT -o "$dbg"
"$dbg"
test -f "$dbg"_cmot_debugsymbols.syms
test -f "$dbg".cmotive.debug.json
grep -q 'debug_level: 3' "$dbg"_cmot_debugsymbols.syms
grep -q 'optimization: O2' "$dbg"_cmot_debugsymbols.syms
grep -q 'StartPackage__DebugThing__Add' "$dbg"_cmot_debugsymbols.syms
build_run tests/conformance/cmotive_sys_io_rename.CMOT sys_io_rename
build_run tests/conformance/cmotive_stl_containers.CMOT stl_containers
build_run tests/conformance/cmotive_algorithms.CMOT algorithms
build_run tests/conformance/cmotive_net_native_sockets.CMOT net_native_sockets
build_run tests/conformance/cmotive_thread_native.CMOT thread_native
build_run tests/conformance/cmotive_dynamic_struct.CMOT dynamic_struct
build_run tests/conformance/cmotive_auto_getset.CMOT auto_getset
build_run tests/conformance/cmotive_operation_overload.CMOT operation_overload
build_run tests/conformance/cmotive_tstore_threadstore.CMOT tstore_threadstore
build_run tests/conformance/cmotive_global_anywhere.CMOT global_anywhere
build_run tests/conformance/cmotive_fptr_function_pointer.CMOT fptr_function_pointer
build_run tests/conformance/cmotive_overridable_pure_virtual.CMOT overridable_pure_virtual
build_run tests/conformance/cmotive_stl_object_methods.CMOT stl_object_methods
build_run tests/conformance/cmotive_algorithms_object_methods.CMOT algorithms_object_methods
build_run tests/conformance/cmotive_io_object_methods.CMOT io_object_methods
build_run tests/conformance/cmotive_filesystem_object_methods.CMOT filesystem_object_methods
build_run tests/conformance/cmotive_net_object_methods.CMOT net_object_methods
build_run tests/conformance/cmotive_thread_object_methods.CMOT thread_object_methods
build_run tests/conformance/cmotive_string_wide_object_methods.CMOT string_wide_object_methods
"$CMOTIVE" --emit-c tests/conformance/cmotive_dynamic_struct.CMOT -o build/dynamic_struct.c
grep -q 'typedef struct MyDynStruct' build/dynamic_struct.c
grep -q 'uint16_t d;' build/dynamic_struct.c
grep -q 'long double i;' build/dynamic_struct.c
if "$CMOTIVE" tests/conformance/cmotive_invalid_base.CMOT -o "build/invalid_base$EXEEXT" >/tmp/cmotive_invalid_base.out 2>/tmp/cmotive_invalid_base.err; then
  echo "expected invalid base compile failure" >&2
  exit 1
fi
if [ "$FULL" = "--full" ]; then
  build_run tests/conformance/cmotive_control_flow.CMTV control
  build_run tests/conformance/cmotive_preprocessor.CMOT preprocessor
  build_run tests/conformance/cmotive_sys_stl_template.CMOT sys_stl_template
  build_run tests/conformance/cmotive_package_method_mangling.CMOT package_method_mangling
  build_run tests/conformance/cmotive_keyword_synonyms.CMOT keyword_synonyms
  build_run tests/conformance/cmotive_exception_destructor_unwind.CMOT exception_destructor_unwind
  build_run tests/conformance/cmotive_constructor_type_overload.CMOT constructor_type_overload
  build_run tests/conformance/cmotive_target_hit_object.CMOT target_hit_object
  build_run tests/conformance/cmotive_target_hit_sender.CMOT target_hit_sender
fi
echo "CMotive tests: PASS"
