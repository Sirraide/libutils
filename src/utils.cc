#include "../include/utils.h"

#include <codecvt>
#include <locale>
#include <cstdarg>

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

std::string Escape(const std::string& str) {
    std::string ret;
    for (auto c : str) {
        switch (c) {
            case '\n': ret += "\\n"; continue;
            case '\r': ret += "\\r"; continue;
            case '\t': ret += "\\t"; continue;
            case '\v': ret += "\\v"; continue;
            case '\f': ret += "\\f"; continue;
            case '\'': ret += "\\\'"; continue;
            case '\"': ret += "\\\""; continue;
            default: ret += c;
        }
    }
    return ret;
}

LIBUTILS_NAMESPACE_END
