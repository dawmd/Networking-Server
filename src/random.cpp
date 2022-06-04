#include "random.h"

#include <chrono>

namespace SK {

std::uint32_t get_seed() {
    return static_cast<std::uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
}

std::uint32_t rand(std::uint32_t seed) {
    constexpr std::uint64_t prime = 2'147'483'647UL;
    constexpr std::uint64_t multiplier = 48'271UL;
    return (static_cast<std::uint64_t>(seed) * multiplier) % prime;
}

} // namespace SK
