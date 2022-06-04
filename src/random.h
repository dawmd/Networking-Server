#ifndef __SK_RANDOM_H__
#define __SK_RANDOM_H__

#include <cstdint>

namespace SK {

std::uint32_t get_seed();
std::uint32_t rand(std::uint32_t seed);

} // namespace SK

#endif // __SK_RANDOM_H__
