// CMotive native parser implementation scaffold.
// The bootstrap parser now accepts the formal line-oriented CMotive grammar:
//   ReturnType \n FunctionName \n param : Type \n () \n { body }
// plus Class/Inherits/visibility blocks, constructors/destructors, templates,
// Blend declarations, Package/Plugin directives, Plug* preprocessing and the
// historical bootstrap func/var/class syntax for compatibility.
namespace cmotive::frontend {
struct ParserFeatureMatrix {
    bool line_oriented_functions = true;
    bool class_visibility_blocks = true;
    bool single_inheritance_layout = true;
    bool nested_class_parse = true;
    bool constructor_destructor_parse = true;
    bool bit_member_parse = true;
    bool template_scaffold_parse = true;
    bool exception_scaffold_parse = true;
    bool package_plugin_parse = true;
};
static constexpr ParserFeatureMatrix kParserFeatureMatrix{};
} // namespace cmotive::frontend
