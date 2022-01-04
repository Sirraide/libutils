#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <iostream>
#include <string>

#ifdef LIBUTILS_NO_GLOBAL_NAMESPACE
#    define LIBUTILS_NAMESPACE_BEGIN namespace utils {
#    define LIBUTILS_NAMESPACE_END   }
#else
#    define LIBUTILS_NAMESPACE_BEGIN
#    define LIBUTILS_NAMESPACE_END
#endif

#define LIBUTILS_CONSTEXPR_NOT_IMPLEMENTED(msg)         \
    []<bool flag = false> { static_assert(flag, msg); } \
    ()

#ifndef LIBUTILS_USE_SCREAMING_SNAKE_CASE
#    define ConstexprNotImplemented(msg) LIBUTILS_CONSTEXPR_NOT_IMPLEMENTED(msg)
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

/**
 * Convert UTF-32 to UTF-8
 *
 * @param what The UTF-32 string to be converted
 * @return A new std::string containing the contents of `what' as a UTF-8 string
 */
std::string ToUTF8(const String& what);

/**
 * Convert UTF-8 to UTF-32
 *
 * @param what The UTF-8 string to be converted
 * @return A new std::string containing the contents of `what' as a UTF-32 string
 */
String ToUTF32(const std::string& what);

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

LIBUTILS_NAMESPACE_END

#endif /* UTILS_H */
