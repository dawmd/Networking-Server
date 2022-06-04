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

} // namespace SK

#endif // __SK_UTILITIES_MISCELLANEOUS_H__
