// x86_64 default native toolchain backend metadata.
//
// The executable bootstrap compiler currently lowers CMotive to C, then invokes
// the platform-native C compiler/linker.  For x86_64 this backend records the
// default path used by tools/cmotive.py:
//   - canonical aliases: x64, amd64, x86_64 -> x86_64
//   - Linux/Unix compile/link flag: -m64
//   - macOS compile/link flag: -arch x86_64
//   - Windows: selected by the active VS/clang x64 developer environment
//
// This file is intentionally small until the direct machine-code backend takes
// over instruction selection and relocation emission.
namespace cmotive::codegen::x86_64 {
struct ToolchainPath {
    const char *canonical_target = "x86_64";
    const char *linux_flag = "-m64";
    const char *darwin_flag = "-arch x86_64";
    bool strip_compatible_symbols = true;
    bool native_object_output = true;
    bool native_executable_output = true;
};
static constexpr ToolchainPath kDefaultNativeToolchainPath{};
} // namespace cmotive::codegen::x86_64
