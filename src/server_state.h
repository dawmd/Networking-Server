#ifndef __SK_SERVER_STATE_H__
#define __SK_SERVER_STATE_H__

#include <messages/common.h>
#include <messages/network_string.h>

#include <optional>
#include <unordered_set>
#include <vector>
#include <utility>  // std::pair

namespace SK {

struct IdleState {
    struct PlayerInfo {
        String name;
        String ip_address;
        std::uint32_t port;
    };

    std::vector<PlayerInfo> players;
    std::uint32_t seed;
};

struct GameState {
    struct TurnInfo {
        struct BombInfo {
            std::pair<u32, u32> position;
            u32 timeout;
        };
        
        std::vector<std::pair<u32, u32>> positions;
        std::vector<u32> death_count;
        std::vector<BombInfo> bombs;
        std::unordered_set<std::pair<u32, u32>> blocks;
    };

    std::vector<TurnInfo> turns;
};

struct ServerState {
    IdleState idle_state;
    GameState game_state;
};

struct ServerParameters {
    u16 bomb_timer;
    u8 player_count;
    u64 turn_durations; // in ms
    u16 explosion_radius;
    u16 initial_blocks; // how many blocks at the beginning of a game
    u16 game_length; // how many turns per game
    String server_name;
    u16 port;
    std::optional<u32> seed;
    u16 size_x;
    u16 size_y;
};

struct Server {
    ServerParameters parameters;
    ServerState state;
};

} // namespace SK

#endif // __SK_SERVER_STATE_H__
