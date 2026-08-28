# CMotive example coverage

## Basics/extensions/preprocessor

- 001: `examples/001_hello_world.CMOT` — program entry point and stdout
- 002: `examples/002_main_args.CMOT` — formal main with argc and argv parameters
- 003: `examples/003_line_comments.CMOT` — single-line comments
- 004: `examples/004_block_comments.CMOT` — block comments
- 005: `examples/005_source_extension_cmtv.CMTV` — CMTV source extension
- 006: `examples/006_lowercase_cmot_extension.cmot` — lowercase cmot source extension
- 007: `examples/007_lowercase_cmtv_extension.cmtv` — lowercase cmtv source extension
- 008: `examples/008_plugin_declaration.CMOT` — Plugin declaration syntax
- 009: `examples/009_package_declaration.CMOT` — Package declaration syntax
- 010: `examples/010_replace_macro.CMOT` — Replace preprocessor value macro

## Types/declarations

- 011: `examples/011_boolean_type.CMOT` — Boolean datatype and True/False
- 012: `examples/012_char_type.CMOT` — Char datatype
- 013: `examples/013_char16_type.CMOT` — Char16 datatype
- 014: `examples/014_char32_type.CMOT` — Char32 datatype
- 015: `examples/015_signed_integer_types.CMOT` — I16/I32/I64 signed integer datatypes
- 016: `examples/016_unsigned_integer_types.CMOT` — Uchar/U16/U32/U64 unsigned datatypes
- 017: `examples/017_floating_types.CMOT` — Float/Double/Ldouble datatypes
- 018: `examples/018_void_pointer.CMOT` — Void pointer and Null
- 019: `examples/019_char_pointer.CMOT` — Char pointer string literal
- 020: `examples/020_fixed_char_array.CMOT` — Char array declaration
- 021: `examples/021_integer_array_initializer.CMOT` — integer array initializer
- 022: `examples/022_contains_string.CMOT` — Contains raw string literal
- 023: `examples/023_const_type.CMOT` — Const datatype qualifier
- 024: `examples/024_volatile_type.CMOT` — Volatile datatype qualifier
- 025: `examples/025_dynamic_type.CMOT` — Dynamic storage scaffold datatype
- 026: `examples/026_struct_keyword_type.CMOT` — Struct keyword scaffold datatype through Dynamic struct-style storage
- 027: `examples/027_global_variable.CMOT` — top-level global variable lowering
- 028: `examples/028_static_function_decorator.CMOT` — Static function decorator scaffold
- 029: `examples/029_inline_function_decorator.CMOT` — Inline function decorator scaffold
- 030: `examples/030_extern_function_decorator.CMOT` — Extern function decorator scaffold

## Operators

- 031: `examples/031_addition_operator.CMOT` — addition operator
- 032: `examples/032_subtraction_operator.CMOT` — subtraction operator
- 033: `examples/033_multiplication_operator.CMOT` — multiplication operator
- 034: `examples/034_division_operator.CMOT` — division operator
- 035: `examples/035_modulo_operator.CMOT` — modulo operator
- 036: `examples/036_comparison_equal.CMOT` — == comparison operator
- 037: `examples/037_comparison_not_equal.CMOT` — != comparison operator
- 038: `examples/038_comparison_less_greater.CMOT` — < and > comparison operators
- 039: `examples/039_comparison_le_ge.CMOT` — <= and >= comparison operators
- 040: `examples/040_logical_and_or.CMOT` — && and || logical operators
- 041: `examples/041_not_operator.CMOT` — Not keyword operator
- 042: `examples/042_bitwise_and_or_xor.CMOT` — bitwise &, |, ^ operators
- 043: `examples/043_bitwise_not.CMOT` — bitwise ~ operator
- 044: `examples/044_left_shift.CMOT` — << left shift operator
- 045: `examples/045_right_shift.CMOT` — >> right shift operator
- 046: `examples/046_right_rotate.CMOT` — >>> right rotate operator scaffold
- 047: `examples/047_left_rotate.CMOT` — <<< left rotate operator scaffold
- 048: `examples/048_compound_add.CMOT` — += compound assignment
- 049: `examples/049_compound_sub.CMOT` — -= compound assignment
- 050: `examples/050_compound_mul.CMOT` — *= compound assignment
- 051: `examples/051_compound_div.CMOT` — /= compound assignment
- 052: `examples/052_increment_operator.CMOT` — ++ increment operator
- 053: `examples/053_decrement_operator.CMOT` — -- decrement operator
- 054: `examples/054_ternary_operator.CMOT` — ?: ternary operator

## Control flow

- 055: `examples/055_sizeof_operator.CMOT` — Sizeof operator
- 056: `examples/056_c_style_cast.CMOT` — C-style CMotive typecast
- 057: `examples/057_address_and_pointer_deref.CMOT` — address-of and pointer dereference operators
- 058: `examples/058_if_else.CMOT` — If/Else control flow
- 059: `examples/059_if_elif_else.CMOT` — If/Elif/Else control flow
- 060: `examples/060_nested_if.CMOT` — nested If blocks
- 061: `examples/061_while_loop.CMOT` — While loop
- 062: `examples/062_do_while_loop.CMOT` — Do/While loop
- 063: `examples/063_for_loop.CMOT` — For loop
- 064: `examples/064_break_loop.CMOT` — Break in loop
- 065: `examples/065_continue_loop.CMOT` — Continue in loop
- 066: `examples/066_switch_case_default.CMOT` — Switch/Case/Default control flow
- 067: `examples/067_return_statement.CMOT` — Return statement
- 068: `examples/068_scoped_block.CMOT` — standalone nested block scaffold

## Functions/decorators

- 069: `examples/069_raw_c_style_expression.CMOT` — C-compatible raw expression statement
- 070: `examples/070_loop_counter_mix.CMOT` — combined While and For loops
- 071: `examples/071_nested_loops.CMOT` — nested loops
- 072: `examples/072_standalone_function_call.CMOT` — line-oriented standalone function declaration and call
- 073: `examples/073_void_function_call.CMOT` — Void function declaration and call
- 074: `examples/074_function_parameters.CMOT` — function parameters
- 075: `examples/075_default_argument_syntax.CMOT` — default argument syntax scaffold
- 076: `examples/076_function_pointer_keyword.CMOT` — Fptr keyword decorator scaffold
- 077: `examples/077_register_decorator.CMOT` — Register keyword decorator scaffold
- 078: `examples/078_tstore_decorator.CMOT` — Tstore keyword decorator scaffold

## Classes/OOP

- 079: `examples/079_overridable_function_decorator.CMOT` — Overridable function decorator scaffold
- 080: `examples/080_multiple_functions.CMOT` — multiple function declarations
- 081: `examples/081_function_returns_bool_style.CMOT` — function returning Boolean
- 082: `examples/082_class_public_field.CMOT` — Class with Public field
- 083: `examples/083_class_private_protected.CMOT` — Private and Protected visibility blocks
- 084: `examples/084_class_constructor.CMOT` — constructor declaration
- 085: `examples/085_class_destructor.CMOT` — destructor declaration
- 086: `examples/086_class_method.CMOT` — method declaration and lowering
- 087: `examples/087_class_method_this_arrow.CMOT` — This pointer in method body
- 088: `examples/088_single_inheritance.CMOT` — single inheritance syntax
- 089: `examples/089_multiple_inheritance_metadata.CMOT` — multiple inheritance metadata scaffold
- 090: `examples/090_overridable_method.CMOT` — Overridable method scaffold
- 091: `examples/091_pure_virtual_method.CMOT` — pure virtual method declaration scaffold
- 092: `examples/092_nested_class.CMOT` — Class declared inside another Class
- 093: `examples/093_new_delete_class_pointer.CMOT` — New/Delete dynamic object allocation
- 094: `examples/094_new_delete_generic.CMOT` — New/Delete generic allocation scaffold
- 095: `examples/095_block_member.CMOT` — Block member marker preventing generated get/set scaffold
- 096: `examples/096_bit_member.CMOT` — bit member specification

## Templates/Blend/Enum/exceptions/includes

- 097: `examples/097_out_of_class_method.CMOT` — out-of-class method implementation with $Class prefix
- 098: `examples/098_camelcase_names.CMOT` — CamelCase class and method naming
- 099: `examples/099_member_access_value.CMOT` — member access with dot syntax from value object scaffold
- 100: `examples/100_member_access_pointer.CMOT` — member access with pointer arrow syntax
- 101: `examples/101_template_class.CMOT` — Template class scaffold
- 102: `examples/102_template_multi_type.CMOT` — Template with multiple Type parameters
- 103: `examples/103_template_function_scaffold.CMOT` — Template function scaffold
- 104: `examples/104_blend_declaration.CMOT` — Blend declaration scaffold
- 105: `examples/105_enum_declaration.CMOT` — Enum declaration scaffold
- 106: `examples/106_try_catchall.CMOT` — Try/Catchall exception scaffold without throwing
- 107: `examples/107_try_catch_typed.CMOT` — Try/Catch typed exception scaffold without throwing
- 108: `examples/108_throw_keyword_scaffold.CMOT` — Throw keyword present in unreachable branch
- 109: `examples/109_plugswitch_os.CMOT` — Plugswitch/Plugcase OS selection
- 110: `examples/110_plugswitch_endian.CMOT` — Plugswitch/Plugcase endian selection

## Standard library/platform/separate compilation

- 111: `examples/111_plugswitch_defined.CMOT` — Plugswitch Defined expression selection
- 112: `examples/112_include_hmot_header.CMOT` — #include .HMOT header
- 113: `examples/113_include_hmtv_header.CMOT` — #include .HMTV header
- 114: `examples/114_include_template_header.CMOT` — #include template header scaffold
- 115: `examples/115_package_plugin_nested_name.CMOT` — nested Package and Plugin names
- 116: `examples/116_stdlib_stdio_print.CMOT` — Sys::Stdio::print lowering
- 117: `examples/117_stdlib_stdio_println.CMOT` — Sys::Stdio::println lowering
- 118: `examples/118_stdlib_cout_write.CMOT` — cout.expect().write() fluent output
- 119: `examples/119_stdlib_logging_info.CMOT` — Sys::Logging::info lowering
- 120: `examples/120_stdlib_filesystem_exists_helper.CMOT` — Sys::Filesystem exists placeholder usage
- 121: `examples/121_stdlib_file_class.CMOT` — Sys::File package class scaffold via direct include
- 122: `examples/122_stdlib_thread_class.CMOT` — Sys::Thread package class scaffold via direct include
- 123: `examples/123_stdlib_net_socket_class.CMOT` — Sys::Net package Socket scaffold via direct include
- 124: `examples/124_stdlib_stl_vector_scaffold.CMOT` — Sys::STL Vector template scaffold via direct include
- 125: `examples/125_stdlib_filesystem_header.CMOT` — Sys::Filesystem header scaffold via direct include
- 126: `examples/126_stdlib_logging_header.CMOT` — Sys::Logging header scaffold via direct include
- 127: `examples/127_thread_local_tstore_syntax.CMOT` — Tstore thread-local syntax scaffold
- 128: `examples/128_network_tcp_udp_raw_terms.CMOT` — network TCP/UDP/raw socket terms as enum scaffold
- 129: `examples/129_filesystem_path_string.CMOT` — filesystem path string handling
- 130: `examples/130_logging_error_unreachable.CMOT` — Sys::Logging::error present in unreachable branch
- 131: `examples/131_target_arch_neutral.CMOT` — target architecture neutral source example
- 132: `examples/132_arm_keyword_coverage.CMOT` — ARM target coverage source marker
- 133: `examples/133_x86_keyword_coverage.CMOT` — x86 target coverage source marker
- 134: `examples/134_x64_keyword_coverage.CMOT` — x86_64 target coverage source marker
- 135: `examples/135_separate_compilation_unit_a.CMOT` — separate compilation source unit A scaffold
- 136: `examples/136_separate_compilation_unit_b.CMOT` — separate compilation source unit B scaffold
- 137: `examples/137_plugin_package_source_shape.CMOT` — package/plugin source shape scaffold

## Scaffold note

Some CMotive requirements are currently represented as compile-safe scaffolds because the compiler/runtime still marks them as scaffolded: full template instantiation, full exception unwinding, package loading, userspace scheduler implementation, real sockets, STL containers, auto Get/Set/Getall/Setall materialization, and Operation operator overloading. Runnable examples still compile, link, execute, and print verification markers.


Added examples 138-145 for Sys::IO rename, STL containers, Sys::Algorithms, native sockets, native threading, and Dynamic Struct Expand.


Additional completed-feature examples added: 146 Global anywhere, 147 Fptr function pointer, 148 Overridable pure virtual, 149 ThreadStore/Tstore.


Updated count: 150 examples. Examples 146-149 cover package-scope Global, Fptr function pointers, Overridable pure virtual declarations, and ThreadStore/Tstore.


- `examples/150_debug_symbols_options.CMOT`: exercises source-level debug symbol metadata with `-g3 -O2` through the CMotive source test suite and can be used manually with `CMotiveSymsToDebugFile`.
