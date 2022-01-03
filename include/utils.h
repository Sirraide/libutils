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

/**
 * Whether a Container supports the operations
 * begin() and end()
 */
template <typename Container, typename Element>
concept Searchable = requires(Container c, Element e) {
    { c.begin() } -> std::forward_iterator;
    { c.end() } -> std::forward_iterator;
    std::is_same_v<decltype(*c.begin()), decltype(e)>;
};

/**
 * Whether a Container supports the operations
 * cbegin() and cend()
 */
template <typename Container, typename Element>
concept CSearchable = requires(Container c, const Element& e) {
    { c.cbegin() } -> std::forward_iterator;
    { c.cend() } -> std::forward_iterator;
    std::is_same_v<decltype(*c.cbegin()), decltype(e)>;
};

/**
 * Find and return an iterator to and element in a collection
 *
 * @param collection The collection to be searched
 * @param element The element to be found
 * @return An iterator to the element found, or collection.end()
 * if the element was not found.
 */
template <typename Element, Searchable<Element> Collection>
auto Find(Collection& collection, const Element& element) -> decltype(collection.begin()) {
    for (auto it = collection.begin(); it != collection.end(); ++it)
        if (*it == element) return it;
    return collection.end();
}

/**
 * Find and return an immutable iterator to and element in a collection
 *
 * @param collection The immutable collection to be searched
 * @param element The element to be found
 * @return A const-iterator to the element found, or collection.cend()
 * if the element was not found.
 */
template <typename Element, CSearchable<Element> Collection>
auto Find(const Collection& collection, const Element& element) -> decltype(collection.cbegin()) {
    for (auto it = collection.cbegin(); it != collection.cend(); ++it)
        if (*it == element) return it;
    return collection.cend();
}

LIBUTILS_NAMESPACE_END

#endif /* UTILS_H */
