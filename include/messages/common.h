#ifndef __SK_MESSAGES_COMMON_H__
#define __SK_MESSAGES_COMMON_H__

#include <cstdint>

namespace SK {

/* Aliases */
using i8    = std::int8_t;
using i16   = std::int16_t;
using i32   = std::int32_t;
using i64   = std::int64_t;

using u8    = std::uint8_t;
using u16   = std::uint16_t;
using u32   = std::uint32_t;
using u64   = std::uint64_t;

/* Custom types */
using BombId    = u32;
using PlayerId  = u8;
using Score     = u32;

} // namespace SK

#endif // __SK_MESSAGES_COMMON_H__
