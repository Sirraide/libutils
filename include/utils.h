#ifndef UTILS_H
#define UTILS_H

#include <codecvt>
#include <cstdlib>
#include <iostream>
#include <locale>
#include <string>
#include <functional>

#ifdef NDEBUG
#    define DEBUG(...)
#else
#    define DEBUG(...) __VA_ARGS__
#endif

#define LIBUTILS_NAMESPACE_BEGIN
#define LIBUTILS_NAMESPACE_END

#define LIBUTILS_CONSTEXPR_NOT_IMPLEMENTED(msg)         \
    []<bool flag = false> { static_assert(flag, msg); } \
    ()

#define LIBUTILS_UNREACHABLE(...)                                                                                                          \
    do {                                                                                                                                   \
        __builtin_unreachable();                                                                                                           \
        std::cerr << __FILE__ << ":" << __LINE__ << ": Error: Unreachable" << __func__ __VA_OPT__(<< "\n\tNote: " << __VA_ARGS__) << "\n"; \
        abort();                                                                                                                           \
    } while (0)

#define LIBUTILS_NON_COPYABLE(type) \
    type(const type&) = delete;     \
    type& operator=(const type&) = delete

#define LIBUTILS_NON_MOVABLE(type) \
    type(type&&)  = delete;        \
    type& operator=(type&&) = delete

#define LIBUTILS_NON_COPYABLE_NON_MOVABLE(type) \
    LIBUTILS_NON_COPYABLE(type);                \
    LIBUTILS_NON_MOVABLE(type)

#ifndef LIBUTILS_USE_SCREAMING_SNAKE_CASE
#    define ConstexprNotImplemented(msg) LIBUTILS_CONSTEXPR_NOT_IMPLEMENTED(msg)
#    define NonCopyable(type)            LIBUTILS_NON_COPYABLE(type)
#    define NonMovable(type)             LIBUTILS_NON_MOVABLE(type)
#    define Unreachable(...)             LIBUTILS_UNREACHABLE(__VA_ARGS__)
#endif

LIBUTILS_NAMESPACE_BEGIN

using Char   = char32_t;
using String = std::u32string;

using U8  = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;

using I8  = int8_t;
using I16 = int16_t;
using I32 = int32_t;
using I64 = int64_t;

using F32 = float;
using F64 = double;

using _err_handler_t = std::function<void(const std::string&)>;

/**
 * This function does what you think it does
 */
[[noreturn]] [[gnu::format(printf, 1, 2)]] //
void Die(const char* format, ...);

/** Escape a string
 *  @param str the string to be escaped
 *  @returns a new string containing the escaped contents of `str`
 */
String Escape(const String& str);

/** Escape a string
 *  @param str the string to be escaped
 *  @returns a new string containing the escaped contents of `str`
 */
std::string Escape(const std::string& str);

/**
 * Convert UTF-32 to UTF-8
 *
 * @param what The UTF-32 string to be converted
 * @return A new std::string containing the contents of `what' as a UTF-8 string
 */
template <typename TString>
std::string ToUTF8(const TString& what) {
    if constexpr (std::is_convertible_v<TString, std::string>) return what;
    else if constexpr (std::is_convertible_v<TString, std::u32string>) {
        std::wstring_convert<std::codecvt_utf8<Char>, Char> conv;
        return conv.to_bytes(what);
    } else ConstexprNotImplemented("ToUTF8 currently only supports u8 and u32 strings");
}

/**
 * Convert UTF-8 to UTF-32
 *
 * @param what The UTF-8 string to be converted
 * @return A new std::string containing the contents of `what' as a UTF-32 string
 */
template <typename TString>
String ToUTF32(const TString& what) {
    if constexpr (std::is_convertible_v<TString, std::string>) {
        std::wstring_convert<std::codecvt_utf8<Char>, Char> conv;
        return conv.from_bytes(what);
    } else if constexpr (std::is_convertible_v<TString, std::u32string>) return what;
    else ConstexprNotImplemented("ToUTF32 currently only supports u8 and u32 strings");
}

/// Base template
template <bool cond, typename _Then, typename _Else>
struct type_if {};

/// Then-branch
template <typename _Then, typename _Else>
struct type_if<true, _Then, _Else> {
    typedef _Then type;
};

/// Else-branch
template <typename _Then, typename _Else>
struct type_if<false, _Then, _Else> {
    typedef _Else type;
};

/**
 * Evaluates to one of two types based on a condition
 * @tparam cond The condition
 * @tparam _Then The type this evaluates to if `cond' is true
 * @tparam _Else The type this evaluates to if `cond' is false
 */

template <bool cond, typename _Then, typename _Else>
using type_if_t = typename type_if<cond, _Then, _Else>::type;

/**
 * Convert a string to lowercase
 * @tparam TString The string type to be used
 * @param tstring The string to convert
 * @returns A copy of `tstring' with each character converted to lowercase
 */
template <typename TString>
TString ToLower(TString tstring) {
    std::transform(tstring.begin(), tstring.end(), tstring.begin(), [](unsigned char c) { return std::tolower(c); });
    return tstring;
}

/**
 * Trim leading and trailing whitespace
 * @tparam TString The string type to be used
 * @param tstring The string to trim
 * @returns A copy of `tstring' with leading and trailing whitespace removed
 */
template <typename TString>
TString Trim(const TString& tstring) requires(std::is_same_v<std::remove_cvref_t<decltype(tstring[0])>, char>) {
    U64 start = 0, end = tstring.size() - 1;
    while (start < end && std::isspace((unsigned char) tstring[start])) start++;
    while (end > start && std::isspace((unsigned char) tstring[end])) end--;
    return tstring.substr(start, end - start + !std::isspace((unsigned char) tstring[end]));
}


/**
 * Print a message to the terminal and terminate
 * @param errmsg The message to print
 */
[[noreturn]] void _libutils_terminate(const std::string& errmsg);
LIBUTILS_NAMESPACE_END

#endif /* UTILS_H */
