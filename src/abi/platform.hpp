// ABI/platform definitions and class/vtable/object layout reservation.
#pragma once
namespace cmotive::abi {
enum class Processor { Arm, Arm64, X86, X86_64, Native };
struct PlatformAbiFeatureMatrix {
    bool arm = true;
    bool arm64 = true;
    bool x86 = true;
    bool x86_64 = true;
    bool macos_arm64_linker_arch_flag = true;
    bool c_abi_interop = true;
    bool object_header_layout_reserved = true;
    bool vtable_layout_reserved = true;
};
static constexpr PlatformAbiFeatureMatrix kPlatformAbiFeatureMatrix{};
} // namespace cmotive::abi
