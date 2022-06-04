#ifndef __SK_MESSAGES_CLIENT_MESSAGES_H__
#define __SK_MESSAGES_CLIENT_MESSAGES_H__

#include "utilities/smart_struct.h"
#include <messages/basic_message.h>
#include <messages/network_string.h>

#include <cstddef>  // std::byte
#include <variant>

namespace SK {

/* Directions */
namespace Dir {

template<std::size_t Id>
using Direction = Message<Id>;

using Up    = Direction<0>;
using Right = Direction<1>;
using Down  = Direction<2>;
using Left  = Direction<3>;

} // namespace Dir

using Direction = std::variant<Dir::Up, Dir::Right, Dir::Down, Dir::Left>;

/* Messages */
using Join          = Message<0, Field<String, "name">>;
using PlaceBomb     = Message<1>;
using PlaceBlock    = Message<2>;
using Move          = Message<3, Field<Direction, "direction">>;

using ClientMessage = std::variant<Join, PlaceBomb, PlaceBlock, Move>;

} // namespace SK

#endif // __SK_MESSAGES_CLIENT_MESSAGES_H__
