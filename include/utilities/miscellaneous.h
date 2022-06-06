#ifndef __SK_UTILITIES_MISCELLANEOUS_H__
#define __SK_UTILITIES_MISCELLANEOUS_H__

#include <algorithm>
#include <cstddef>

namespace SK {

template<typename T, std::size_t Size = sizeof(T)>
T swap_endiannes(T element) {
    std::byte *bytes = reinterpret_cast<std::byte*>(&element);
    std::reverse(bytes, bytes + Size);
    return *reinterpret_cast<T*>(bytes);
}

template<typename E>
constexpr auto to_underlying(E e) {
    return static_cast<std::underlying_type_t<E>>(e);
}

} // namespace SK

#endif // __SK_UTILITIES_MISCELLANEOUS_H__
