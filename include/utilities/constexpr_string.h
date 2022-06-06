#ifndef __SK_UTILITIES_CONSTEXPR_STRING_H__
#define __SK_UTILITIES_CONSTEXPR_STRING_H__

#include <cstddef>  // std::size_t

namespace SK {

template<std::size_t Length>
    requires (Length > 1)
struct ConstexprString {
    char content[Length];

    consteval ConstexprString(const char (&characters)[Length]) {
        for (std::size_t i = 0; i < Length; ++i)
            content[i] = characters[i];
    }

    consteval bool operator==(const ConstexprString&) const = default;
};

template<std::size_t Length_1, std::size_t Length_2>
consteval bool operator==(const ConstexprString<Length_1>&, const ConstexprString<Length_2>&) {
    return false;
}

} // namespace SK

#endif // __SK_UTILITIES_CONSTEXPR_STRING_H__
