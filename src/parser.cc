#include "../include/parser.h"

LIBUTILS_NAMESPACE_BEGIN

template <typename _SourceFile, typename _Token, typename _SourceLocation, bool _newline_is_token, bool is_32>
_Token LexerBase<_SourceFile, _Token, _SourceLocation, _newline_is_token, is_32>::global_empty_token{};

std::ostream& operator<<(std::ostream& stream, const SourceLocationBase<>& loc) {
    stream << loc.source_file->name << ":" << loc.line << ":" << loc.col;
    return stream;
}

template <typename TChar>
std::basic_string<TChar> StringiseTypeConvertTo(auto str) {
    if constexpr (std::is_same_v<TChar, char32_t>) return ToUTF32(str);
    else if constexpr (std::is_same_v<TChar, char8_t>) return ToUTF8(str);
    else ConstexprNotImplemented("StringiseTypeConvertTo");
}

LIBUTILS_NAMESPACE_END