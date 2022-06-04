#ifndef __SK_MESSAGES_SERVER_MESSAGES_H__
#define __SK_MESSAGES_SERVER_MESSAGES_H__

#include <messages/basic_message.h>
#include <messages/common.h>
#include <messages/network_list.h>
#include <messages/network_map.h>
#include <messages/network_string.h>

#include <variant>

namespace SK {

/* Custom types */
using Position = BasicMessage<
    Field<u16, "x">,
    Field<u16, "y">
>;

using Bomb = BasicMessage<
    Field<Position, "position">,
    Field<u16, "timer">
>;

using Player = BasicMessage<
    Field<String, "name">,
    Field<String, "address">
>;

/* Events */
namespace Event {

using BombPlaced = Message<
    0,
    Field<BombId, "id">,
    Field<Position, "position">
>;

using BombExploded = Message<
    1,
    Field<BombId, "id">,
    Field<List<PlayerId>, "robots_destroyed">,
    Field<List<Position>, "blocks_destroyed">
>;

using PlayerMoved = Message<
    2,
    Field<PlayerId, "id">,
    Field<Position, "position">
>;

using BlockPlaced = Message<
    3,
    Field<Position, "position">
>;

using Event = std::variant<BombPlaced, BombExploded, PlayerMoved, BlockPlaced>;

} // namespace Event

/* Server messages */
using Hello = Message<
    0,
    Field<String, "server_name">,
    Field<u8, "players_count">,
    Field<u16, "size_x">,
    Field<u16, "size_y">,
    Field<u16, "game_length">,
    Field<u16, "explosion_radius">,
    Field<u16, "bomb_timer">
>;

using AcceptedPlayer = Message<
    1,
    Field<Player, "player">
>;

using GameStarted = Message<
    2,
    Field<Map<PlayerId, Player>, "players">
>;

using Turn = Message<
    3,
    Field<u16, "turn">,
    Field<List<Event::Event>, "events">
>;

using GameEnded = Message<
    4,
    Field<Map<PlayerId, Score>, "scores">
>;

using ServerMessage = std::variant<Hello, AcceptedPlayer, GameStarted, Turn, GameEnded>;

} // namespace SK

#endif // __SK_MESSAGES_SERVER_MESSAGES_H__
