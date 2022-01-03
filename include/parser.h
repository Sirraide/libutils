#ifndef UTILS_PARSER_H
#define UTILS_PARSER_H

#include "./unicode-utils.h"
#include "./utils.h"

#include <cstring>
#include <cwctype>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <stack>
#include <sys/mman.h>
#include <sys/stat.h>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <vector>

LIBUTILS_NAMESPACE_BEGIN
#define LEXER_ERROR(format, ...)                              \
    do {                                                      \
        Error(token->loc, format __VA_OPT__(, ) __VA_ARGS__); \
        SkipToEOL();                                          \
        return;                                               \
    } while (0)

#define ESCAPE_CHAR(c, repl, len)      \
    case c:                            \
        token->string_content += repl; \
        goto next_char

constexpr inline Char Eof = (Char(EOF));

static inline U64 XDigitToNumber(Char c) {
    return iswdigit(static_cast<wint_t>(c))
               ? static_cast<U64>(c - L'0')
               : static_cast<U64>(c - L'A' + 10);
}

static inline int iswbdigit(wint_t c) {
    return c == L'0' || c == L'1';
}

static inline int iswodigit(wint_t c) {
    return L'0' <= c && c <= L'7';
}

/** The type of the token */
enum struct TokenTypeBase : Char {
    EndOfFile,
    Identifier = 1000,
    Number,
    LBrace = L'{',
    RBrace = L'}',
    Comma  = L',',
    Colon  = L':',
    String = L'"',
};

struct FileBase {
    String      contents;       /// The contents of the file, mapped into memory
    std::string name;           /// The path to the file
    Char*       end  = nullptr; /// The end of `contents'
    Char*       pos  = nullptr; /// The current position of the lexer in the file
    U64         line = 1;       /// The current line position of the lexer in this file
    U64         col{};          /// The current column position of the lexer in this file
    bool        valid = false;  /// Whether this file is valid or not

    /** Create a new file from the given path
     * @param _name The path to the file */
    explicit FileBase(std::string _name) : name(std::move(_name)) {
        int fd = ::open(name.c_str(), O_RDONLY);
        if (fd < 0) Die("open(): %s", ::strerror(errno));

        struct stat s {};
        if (::fstat(fd, &s)) Die("fstat(): %s", ::strerror(errno));
        auto sz = U64(s.st_size);

        auto* mem = (char*) ::mmap(nullptr, sz,
            PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        if (mem == MAP_FAILED) Die("mmap(): %s", ::strerror(errno));

        contents = ToUTF32(mem);
        if (::munmap(mem, sz)) Die("munmap(): %s", ::strerror(errno));
        if (::close(fd)) Die("close(): %s", ::strerror(errno));

        pos = &contents[0];
        end = contents.data() + contents.size();
    }
};

template <typename File = FileBase>
struct SourceLocationBase {
    U64   line{};                /// The line location
    U64   col{};                 /// The column location
    File* source_file = nullptr; /// Non-owning pointer to the file this location is in

    SourceLocationBase(){};

    template <typename StringType>
    operator StringType() const {
        std::string s;
        s += source_file->name;
        s += ":";
        s += std::to_string(line);
        s += ":";
        s += std::to_string(col);
        if constexpr (std::is_convertible_v<StringType, String>) return ToUTF32(s);
        else if constexpr (std::is_convertible_v<StringType, std::string>) return s;
        else {
            ConstexprNotImplemented("Implicit conversion of SourceLocationBase to anything other than "
                                    "std::u32string or std::string is not supported.");
        }
    }
};

std::ostream& operator<<(std::ostream& stream, const SourceLocationBase<>& loc);

/** A token that is lexed by the lexer. */
template <
    typename TokType        = TokenTypeBase,
    typename SourceLocation = SourceLocationBase<FileBase>>
struct TokenBase {
    using Type = TokType;
    Type           type;           /// The type of this token
    String         string_content; /// The string content of the token
    U64            number{};       /// The number that the token represents, if any
    SourceLocation loc{};          /// Where the token was lexed

    TokenBase(){};

    /**
     * Print a token to stdout
     * @see #String
     */
    void Print(std::ostream& stream = std::cout) const {
        stream << ToUTF8(String());
    }

    /**
     * Stringise this token
     * @return A string representation of this token
     */
    [[nodiscard]] String String() const {
        return StringiseType(this);
    }
};

template <>
String StringiseType(const auto* token);

template <
    typename _File           = FileBase,
    typename _SourceLocation = SourceLocationBase<_File>,
    typename _Token          = TokenBase<TokenTypeBase, _File>,
    bool _newline_is_token   = false>
struct LexerBase;

template <>
String StringiseType<>(const TokenBase<>* token);

template <
    typename _File,
    typename _SourceLocation,
    typename _Token,
    bool _newline_is_token>
struct LexerBase {
    using File           = _File;
    using SourceLocation = _SourceLocation;
    using Token          = _Token;
    using T              = typename Token::Type;

    Char              lastc  = U' ';                        /// The last character read. This is initially a space to trigger a call to SkipWhitespace()
    bool              at_eof = false;                       /// Whether the lexer has reached the end of the file
    Token*            token{};                              /// The last token read
    std::vector<File> files;                                /// All files that were at any point part of the file stack
    std::stack<File*> file_stack;                           /// The file stack
    File*             curr_file{};                          /// The file currently being processed
    bool              newline_is_token = _newline_is_token; /// Whether newlines count as tokens
    bool              has_error        = false;             /// Whether an error has occurred during lexing
    static Token      global_empty_token;

    explicit LexerBase(const std::string& filename) {
        files.emplace_back(filename);
        file_stack.push(&files[0]);
        curr_file = &files[0];
    }

    /**
     * Print the current location and a message and exit
     *
     * @param where The location of the error
     * @param msgs The error messages to print
     */
    [[noreturn]] void Fatal(const SourceLocation& where, const char* format, auto... args) {
        std::cerr << where << ": ";
        Die(format, std::forward<decltype(args)>(args)...);
    }

    /**
     * Print the current location and a message and set the error flag
     *
     * @param where The location of the error
     * @param msgs The error messages to print
     */
    [[gnu::format(printf, 3, 4)]] void Error(const SourceLocation& where, const char* format, ...) {
        has_error = true;
        std::cerr << where << ": ";
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);
        std::cerr << '\n';
    }

    /**
     * Perform the process of lexing a number
     *
     * @param base The base of the number to be lexed
     * @param predicate A predicate called on each character that
     * determines whether it is allowed in that number or not
     */
    virtual void DoLexNumber(U64 base, const std::function<int(wint_t)>& predicate) {
        /// Read all the digits into a buffer
        String number_str;
        while (predicate(wint_t(lastc))) {
            number_str += lastc;
            NextChar();
        }

        /// Convert the string to a number
        U64 number = 0;
        for (Char *ptr = number_str.data(), *end = ptr + number_str.size(); ptr < end; ptr++) {
            /// Multiply and check for overflow
            U64 old_num = number;
            number *= base;
            if (old_num > number) LEXER_ERROR("Literal exceeds maximum integer size");

            /// Add and check for overflow
            old_num = number;
            number += XDigitToNumber(*ptr);
            if (old_num > number) LEXER_ERROR("Literal exceeds maximum integer size");
        }

        token->number = number;
    }

    /**
     * Get the current location
     * @return The current source location of the lexer
     * @see SourceLocation
     */
    [[nodiscard]] SourceLocation Here() const {
        SourceLocation loc;
        loc.source_file = curr_file;
        loc.col         = curr_file->col;
        loc.line        = curr_file->line;
        return loc;
    }

    /** Lex a number at the current position */
    void LexNumber() {
        /// If the number starts with a leading 0, it
        /// could be a binary, octal, or hexadecimal number
        if (lastc == L'0') {
            /// Discard all leading 0s
            do NextChar();
            while (lastc == L'0');

            /// It's hexadecimal
            if (lastc == L'x' || lastc == L'X') {
                /// 0x alone is illegal
                Char x = lastc;
                NextChar();
                if (!iswxdigit(wint_t(lastc)))
                    LEXER_ERROR("Expected at least 1 digit after '0%lc'", x);
                DoLexNumber(16, iswxdigit);
                return;
            }

            /// It's binary
            if (lastc == L'b' || lastc == L'B') {
                /// 0b alone is illegal
                Char b = lastc;
                NextChar();
                if (!iswbdigit(wint_t(lastc)))
                    LEXER_ERROR("Expected at least 1 digit after '0%lc'", b);
                DoLexNumber(2, iswbdigit);
                return;
            }

            /// It's octal
            if (iswodigit(wint_t(lastc))) {
                DoLexNumber(8, iswodigit);
                return;
            }

            /// The number might be 0
            if (!iscontinue(int(lastc)) || lastc == L'-') {
                token->number = 0;
                return;
            }

            /// Anything else is invalid
            LEXER_ERROR("Unexpected character '%lc' in number literal", lastc);
        }

        /// If the number doesn't start with a leading 0,
        /// it's a decimal number
        DoLexNumber(10, iswdigit);
    }

    /** Lex a string literal at the current position */
    virtual void LexStringLiteral(bool yeet_terminator = true) {
        const Char term = lastc; /// " or '
        NextChar();              /// yeet " or '
        if (at_eof) goto unterminated;

        /// Handle '-quoted strings
        if (term == L'\'') {
            while (!at_eof && lastc != L'\'') {
                token->string_content += lastc;
                NextChar();
            }
            if (at_eof && lastc != '\'') goto unterminated;
            if (yeet_terminator) NextChar(); /// yeet '
            return;
        }

        /// Handle "-quoted strings
        while (!at_eof && lastc != L'"') {
            if (lastc == '\\') {
                NextChar();
                if (at_eof && lastc == Eof) goto unterminated;
                switch (lastc) {
                    ESCAPE_CHAR(U'n', U"\n", 1);
                    ESCAPE_CHAR(U'r', U"\r", 1);
                    ESCAPE_CHAR(U'f', U"\f", 1);
                    ESCAPE_CHAR(U'v', U"\v", 1);
                    ESCAPE_CHAR(U'e', U"\033", 1);
                    ESCAPE_CHAR(U'\\', U"\\", 1);
                    ESCAPE_CHAR(U'\'', U"\'", 1);
                    ESCAPE_CHAR(U'\"', U"\"", 1);
                    default:
                        Error(token->loc, "Invalid escape sequence '\\%lc'", lastc);
                        break;
                }
            }
            token->string_content += lastc;
        next_char:
            NextChar();
        }

        if (at_eof && lastc != L'"') goto unterminated;
        if (yeet_terminator) NextChar(); /// yeet "
        return;

    unterminated:
        LEXER_ERROR("unterminated string literal");
    }

    /** Advance by one character */
    virtual void NextChar() {
        /// Pop the current file from the file stack once its end has been reached
        if (at_eof || curr_file->pos == curr_file->end) {
            if (file_stack.empty()) goto do_eof;

            file_stack.pop();
            if (!file_stack.empty()) {
                curr_file = file_stack.top();
                at_eof    = false;
            } else {
            do_eof:
                at_eof = true;
                lastc  = Eof;
                return;
            }
        }

        Char c = *curr_file->pos++;
        curr_file->col++;
        if (curr_file->pos == curr_file->end) {
            lastc = c;
            return;
        }

        /// Handle newlines
        if (c == L'\n' || c == L'\r') {
            curr_file->col = 0;
            curr_file->line++;
            Char c2 = *curr_file->pos++;
            if (c2 == L'\n' || c2 == L'\r') {
                if (c == c2) curr_file->pos--;
            } else curr_file->pos--;
            lastc = L'\n';
            return;
        }

        lastc = c;
    }

    /** Read the next token */
    virtual void NextToken() = 0;

    /** Lex all tokens and print them */
    virtual void PrintAllTokens() {
        do {
            token->Print();
            NextToken();
        } while (token->type != T::EndOfFile);
    }

    /**
     * Include a file
     * <p>
     * This function calls this->ResolveInclude(filename) to
     * find and map the file.
     * @param filename The path to the file to be included
     */
    virtual void IncludeFile(std::string& filename) {
        files.push_back(ResolveInclude(filename));
        file_stack.push(&files.back());
        curr_file = file_stack.top();
    }

    /**
     * Resolve the string content of the current token to a
     * path and return it as a file
     *
     * @return A File created from the path in question
     */
    [[nodiscard]] virtual File ResolveInclude(std::string& filename) const {
        std::string include(curr_file->name);
        include += "/";
        include += filename;
        return File{include};
    }

    /** Advance the current position to the end of the line */
    virtual void SkipToEOL() {
        while (lastc != L'\n' && !at_eof) NextChar();
        NextToken();
    }
    /**
     * Skip all whitespace up to the next token
     * <p>
     * This function takes care not to skip newlines
     * if they are being processed as tokens
     */
    virtual void SkipWhitespace() {
        if (newline_is_token)
            while (!at_eof && lastc != L'\n' && iswspace(wint_t(lastc))) NextChar();
        else
            while (!at_eof && iswspace(wint_t(lastc))) NextChar();
    }

    /**
     * Read characters until a certain character is found
     * <p>
     * When this function returns, the parser is at the
     * first occurrence of the character in question starting
     * from the position it is currently at.
     * <p>
     * If the character is not found, the parser reads until EOF.
     * <p>
     * The character is neither included in the string returned
     * nor discarded.
     * <p>
     * The character that the parser is currently at when ReadUntilChar()
     * is called is included in the string returned
     *
     * @param c The character up to which to read
     * @return A String containing the characters read
     */
    virtual String ReadUntilChar(Char c) {
        String s;
        while (!at_eof && lastc != c) {
            s += lastc;
            NextChar();
        }
        return s;
    }

    /**
     * Read characters until one of the characters supplied is found
     * <p>
     * When this function returns, the parser is at the position of
     * whatever character of the characters supplied is found first
     * from the position it is currently at.
     * <p>
     * If none of the characters are found, the parser reads until EOF.
     * <p>
     * The character found is neither included in the string returned
     * nor discarded.
     * <p>
     * The character that the parser is currently at when ReadUntilChar()
     * is called is included in the string returned
     *
     * @param chars The characters up to which to read
     * @return A String containing the characters read
     */
    String ReadUntilChar(const std::vector<Char>& chars) {
        String s;
        while (!at_eof && std::find(chars.begin(), chars.end(), lastc) == chars.end()) {
            s += lastc;
            NextChar();
        }
        return s;
    }
};
LIBUTILS_NAMESPACE_END

#endif /// UTILS_PARSER_H
