// CMotive AST node model scaffold for the native compiler track.
// The bootstrap AST models Program, Function, ClassDecl, Field, template,
// package/plugin, control-flow, exception, variable, and raw nodes. ClassDecl
// carries a single effective base, full base metadata for diagnostics, fields,
// methods, nested classes and virtual-layout markers. Function nodes mark
// constructors/destructors so codegen can mangle lifecycle functions.
namespace cmotive::ast {
struct AstFeatureMatrix {
    bool has_class_decl = true;
    bool has_field_decl = true;
    bool has_method_decl = true;
    bool marks_constructors = true;
    bool marks_destructors = true;
    bool records_single_inheritance_base = true;
    bool records_nested_classes = true;
};
static constexpr AstFeatureMatrix kAstFeatureMatrix{};
} // namespace cmotive::ast
