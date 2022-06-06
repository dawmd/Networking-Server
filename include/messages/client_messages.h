#ifndef __SK_MESSAGES_CLIENT_MESSAGES_H__
#define __SK_MESSAGES_CLIENT_MESSAGES_H__

#include <messages/message.h>
#include <messages/network_string.h>

namespace SK {

/* Directions */
namespace DirectionMessage {

using Up    = Message<0>;
using Right = Message<1>;
using Down  = Message<2>;
using Left  = Message<3>;

} // namespace DirectionMessage

using Direction = MessageWrapper<
    DirectionMessage::Up,
    DirectionMessage::Right,
    DirectionMessage::Down,
    DirectionMessage::Left
>;

/* Messages */
using Join          = Message<0, Field<String, "name">>;
using PlaceBomb     = Message<1>;
using PlaceBlock    = Message<2>;
using Move          = Message<3, Field<Direction, "direction">>;

using ClientMessage = MessageWrapper<Join, PlaceBomb, PlaceBlock, Move>;

} // namespace SK

#endif // __SK_MESSAGES_CLIENT_MESSAGES_H__
