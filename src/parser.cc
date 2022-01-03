#include "../include/parser.h"

LIBUTILS_NAMESPACE_BEGIN

template <
    typename File,
    typename SourceLocation,
    typename Token,
    bool _newline_is_token>
Token LexerBase<File, SourceLocation, Token, _newline_is_token>::global_empty_token{};

std::ostream& operator<<(std::ostream& stream, const SourceLocationBase<>& loc) {
    stream << loc.source_file->name << ":" << loc.line << ":" << loc.col;
    return stream;
}

template <>
String StringiseType<>(const TokenBase<>* token) {
    using Type = TokenTypeBase;
    switch (token->type) {
        case Type::EndOfFile: return U"[EOF]\n";
        case Type::LBrace:
        case Type::RBrace:
        case Type::Comma:
        case Type::Colon: {
            ::String s;
            s += (Char) token->type;
            s = U"[Token:          " + Escape(s) + U"]\n";
            return s;
        }
        case Type::Identifier:
            return ::String(U"[Identifier:     ") + token->string_content + U"]\n";
        case Type::String: {
            ::String s(U"[String Literal: ");
            s += (Char) token->type;
            s += Escape(token->string_content);
            s += (Char) token->type;
            return s + U"\n";
        }
        case Type::Number:
            return ::String(U"[Number:         ") + ToUTF32(std::to_string(token->number)) + U"]\n";
        default: return U"[INVALID]\n";
    }
}

LIBUTILS_NAMESPACE_END