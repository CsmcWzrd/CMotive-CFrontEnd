// Package/plugin discovery and loading model used by the native C frontend.
//
// Plugin Foo::Bar resolves to Foo/Bar.HMOT, Foo/Bar.HMTV, Foo/Bar.CMOT, or
// Foo/Bar.CMTV, and also to Foo/Bar/package.<ext>. The native C frontend
// materializes the package into the current translation unit before parsing.
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cmotive::packages {
static const char *kPackageExtensions[] = {
    ".HMOT", ".HMTV", ".CMOT", ".CMTV", ".hmot", ".hmtv", ".cmot", ".cmtv"
};

static std::filesystem::path logical_to_path(std::string logical) {
    std::filesystem::path out;
    std::size_t start = 0;
    while (start < logical.size()) {
        std::size_t pos = logical.find("::", start);
        out /= logical.substr(start, pos == std::string::npos ? std::string::npos : pos - start);
        if (pos == std::string::npos) break;
        start = pos + 2;
    }
    return out;
}

std::optional<std::filesystem::path> resolve_plugin(
    const std::string &logical_name,
    const std::vector<std::filesystem::path> &search_paths) {
    const std::filesystem::path rel = logical_to_path(logical_name);
    for (const auto &base : search_paths) {
        for (const char *ext : kPackageExtensions) {
            std::filesystem::path direct = base / (rel.string() + ext);
            if (std::filesystem::exists(direct)) return direct;
            std::filesystem::path package = base / rel / (std::string("package") + ext);
            if (std::filesystem::exists(package)) return package;
            std::filesystem::path package_caps = base / rel / (std::string("Package") + ext);
            if (std::filesystem::exists(package_caps)) return package_caps;
        }
    }
    return std::nullopt;
}

struct PackageFeatureMatrix {
    bool package_declaration = true;
    bool plugin_directive = true;
    bool hmot_hmtv_header_inputs = true;
    bool package_search_path = true;
    bool manifest_scaffold = true;
    bool translation_unit_materialization = true;
};
static constexpr PackageFeatureMatrix kPackageFeatureMatrix{};
} // namespace cmotive::packages
