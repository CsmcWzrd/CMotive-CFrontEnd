// CMotive native lexer implementation scaffold.
// The Python bootstrap lexer in src/cmotive/lexer.py is currently executable.
// This C++ source captures the same formal token contract for the native port.
#include <array>
#include <string_view>

namespace cmotive::frontend {
struct KeywordEntry { std::string_view spelling; std::string_view token; };
static constexpr std::array<KeywordEntry, 76> kKeywords{{
    {"Blend","BLEND"},{"Boolean","BOOLEAN"},{"Break","BREAK"},{"Block","BLOCK"},
    {"Case","CASE"},{"Catch","CATCH"},{"Catchall","CATCHALL"},{"Char","CHAR"},
    {"Char16","CHAR16"},{"Char32","CHAR32"},{"Class","CLASS"},{"Const","CONST"},
    {"Continue","CONTINUE"},{"Contains","CONTAINS"},{"Default","DEFAULT"},{"Delete","DELETE"},
    {"Do","DO"},{"Double","DOUBLE"},{"Dynamic","DYNAMIC"},{"Elif","ELIF"},
    {"Else","ELSE"},{"Enum","ENUM"},{"Extern","EXTERN"},{"False","FALSE"},
    {"Float","FLOAT"},{"For","FOR"},{"Fptr","FPTR"},{"Get","GET"},
    {"Getall","GETALL"},{"Global","GLOBAL"},{"Goto","GOTO"},{"I16","I16"},
    {"I32","I32"},{"I64","I64"},{"If","IF"},{"Inherits","INHERITS"},
    {"Inline","INLINE"},{"Ldouble","LDOUBLE"},{"Package","PACKAGE"},{"New","NEW"},
    {"Not","NOT"},{"Null","NULL"},{"Operation","OPERATION"},{"Overridable","OVERRIDABLE"},
    {"Plugin","PLUGIN"},{"Plugcase","PLUGCASE"},{"Plugdefault","PLUGDEFAULT"},{"Plugswitch","PLUGSWITCH"},
    {"Plugend","PLUGEND"},{"Private","PRIVATE"},{"Protected","PROTECTED"},{"Public","PUBLIC"},
    {"Register","REGISTER"},{"Replace","REPLACE"},{"Return","RETURN"},{"Set","SET"},
    {"Setall","SETALL"},{"Sizeof","SIZEOF"},{"Static","STATIC"},{"Struct","STRUCT"},
    {"Switch","SWITCH"},{"Template","TEMPLATE"},{"Throw","THROW"},{"This","THIS"},
    {"Tstore","TSTORE"},{"True","TRUE"},{"Try","TRY"},{"Type","TYPE"},
    {"Uchar","UCHAR"},{"U16","U16"},{"U32","U32"},{"U64","U64"},
    {"Void","VOID"},{"Volatile","VOLATILE"},{"While","WHILE"},{"Operation","OPERATION"}
}};
static constexpr std::array<std::string_view, 34> kOperators{{
    "New","Delete","+","-","*","/","%","=","==","!=","<",">","<=",">=",
    "&&","||","!","~","&","|","^","<<",">>",">>>","<<<","++","--",
    "+=","-=","*=","/=","%=","->","."
}};
} // namespace cmotive::frontend
