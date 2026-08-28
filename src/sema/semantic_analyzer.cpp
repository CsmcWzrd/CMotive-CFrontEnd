// CMotive semantic analysis implementation notes for the native compiler track.
// The executable bootstrap compiler in src/cmotive/semantic.py now performs the
// same checks described here: duplicate classes/globals, single inheritance,
// existing-base validation, inheritance-cycle detection, duplicate method
// signatures, and constructor/destructor name validation.
namespace cmotive::sema {
struct SemanticFeatureMatrix {
    bool validates_duplicate_classes = true;
    bool validates_single_inheritance = true;
    bool validates_base_exists = true;
    bool rejects_inheritance_cycles = true;
    bool validates_constructor_names = true;
    bool validates_destructor_names = true;
};
static constexpr SemanticFeatureMatrix kSemanticFeatureMatrix{};
} // namespace cmotive::sema
