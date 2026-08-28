// Object metadata, symbols, and separate compilation scaffold.
// cmotive -c emits native .o/.obj files.  Linking one or more generated objects
// through the platform linker/driver is supported by tools/cmotive.py.
namespace cmotive::separate_compilation {
struct ObjectModelFeatureMatrix {
    bool compile_only_objects = true;
    bool link_objects_to_executable = true;
    bool symbol_names_are_strip_compatible = true;
    bool method_name_mangling_scaffold = true;
    bool package_metadata_placeholder = true;
};
static constexpr ObjectModelFeatureMatrix kObjectModelFeatureMatrix{};
} // namespace cmotive::separate_compilation
