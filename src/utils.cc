#include "../include/utils.h"

#include <codecvt>
#include <locale>

LIBUTILS_NAMESPACE_BEGIN

[[noreturn]] [[gnu::format(printf, 1, 2)]] //
void Die(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "\033[1;31mError:\033[1;39m ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\033[m\n");
    va_end(ap);
    exit(1);
}

std::string ToUTF8(const String& what) {
    std::wstring_convert<std::codecvt_utf8<Char>, Char> conv;
    return conv.to_bytes(what);
}

String ToUTF32(const std::string& what) {
    std::wstring_convert<std::codecvt_utf8<Char>, Char> conv;
    return conv.from_bytes(what);
}

String Escape(const String& str) {
    String ret;
    for (auto c : str) {
        switch (c) {
            case U'\n': ret += U"\\n"; continue;
            case U'\r': ret += U"\\r"; continue;
            case U'\t': ret += U"\\t"; continue;
            case U'\v': ret += U"\\v"; continue;
            case U'\f': ret += U"\\f"; continue;
            case U'\'': ret += U"\\\'"; continue;
            case U'\"': ret += U"\\\""; continue;
            default: ret += c;
        }
    }
    return ret;
}

LIBUTILS_NAMESPACE_END
