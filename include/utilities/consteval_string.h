#ifndef __SK_UTILITIES_CONSTEVAL_STRING_H__
#define __SK_UTILITIES_CONSTEVAL_STRING_H__

#include <cstddef>  // std::size_t

namespace SK {

template<std::size_t Length>
    requires (Length > 1)
struct ConstevalString {
    char content[Length];

    consteval ConstevalString(const char (&characters)[Length]) {
        for (std::size_t i = 0; i < Length; ++i)
            content[i] = characters[i];
    }

    consteval bool operator==(const ConstevalString&) const = default;
};

template<std::size_t Length_1, std::size_t Length_2>
consteval bool operator==(const ConstevalString<Length_1>&, const ConstevalString<Length_2>&) {
    return false;
}

} // namespace SK

#endif // __SK_UTILITIES_CONSTEVAL_STRING_H__
