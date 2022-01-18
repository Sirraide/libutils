#include "../include/parser.h"

LIBUTILS_NAMESPACE_BEGIN

template <
    typename File,
    typename SourceLocation,
    typename Token,
    bool _newline_is_token,
    bool is_32>
Token LexerBase<File, SourceLocation, Token, _newline_is_token, is_32>::global_empty_token{};

std::ostream& operator<<(std::ostream& stream, const SourceLocationBase<>& loc) {
    stream << loc.source_file->name << ":" << loc.line << ":" << loc.col;
    return stream;
}

template <>
std::string StringiseType<>(const TokenBase<>* token) {
    using Type = TokenTypeBase;
    switch (token->type) {
        case Type::EndOfFile: return "[EOF]\n";
        case Type::LBrace:
        case Type::RBrace:
        case Type::Comma:
        case Type::Colon: {
            std::string s;
            s = (char) token->type;
            s += "[Token:          ";
            s += Escape(s);
            s += "]\n";
            return s;
        }
        case Type::Identifier:
            return "[Identifier:     " + ToUTF8(token->string_content) + "]\n";
        case Type::String: {
            std::string s = "[String Literal: ";
            s += (char) token->type;
            s += Escape(ToUTF8(token->string_content));
            s += (char) token->type;
            return s + "\n";
        }
        case Type::Number:
            return "[Number:         " + std::to_string(token->number) + "]\n";
        default: return "[INVALID]\n";
    }
}

LIBUTILS_NAMESPACE_END