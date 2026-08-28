// CMotive template instantiation model.
//
// The executable bootstrap compiler implements concrete class/function template
// instantiation in src/cmotive/codegen.py.  This native-side file documents and
// mirrors the ABI contract used by the current compiler path: template symbols
// are materialized only when a concrete use such as Box<I32> or Identity<I32>(x)
// is encountered, and the generated symbol name is Template__Arg0__ArgN.
#include <string>
#include <vector>

namespace cmotive::templates {
struct TemplateParameter {
    std::string name;
    std::string kind;
};

struct TemplateInstance {
    std::string template_name;
    std::vector<std::string> arguments;
    std::string symbol_name;
};

static std::string sanitize_for_symbol(std::string value) {
    for (char &ch : value) {
        const bool ok = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                        (ch >= '0' && ch <= '9') || ch == '_';
        if (!ok) ch = '_';
    }
    return value;
}

std::string make_instance_symbol(const std::string &template_name,
                                 const std::vector<std::string> &arguments) {
    std::string out = sanitize_for_symbol(template_name);
    for (const std::string &arg : arguments) {
        out += "__";
        out += sanitize_for_symbol(arg);
    }
    return out;
}

TemplateInstance instantiate(const std::string &template_name,
                             const std::vector<std::string> &arguments) {
    return TemplateInstance{template_name, arguments,
                            make_instance_symbol(template_name, arguments)};
}
} // namespace cmotive::templates
