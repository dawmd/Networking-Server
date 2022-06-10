#ifndef __SK_MESSENGER_H__
#define __SK_MESSENGER_H__

#include <messages/client_messages.h>
#include <messages/network_string.h>
#include <messages/server_messages.h>
#include <network/socket.h>
#include <stdexcept>
#include <thread>
#include <utilities/monitor.h>
#include <utilities/thread_pool.h>

#include <array>
#include <cstddef>
#include <cstring>  // std::memmove
#include <functional>
#include <optional>
#include <vector>

#include "auxiliary.h"

#define GET_FIELD(name) get<#name>()

namespace SK {

struct ClientInfo {
    TCPSocket socket;
    String name;
    String address;

    ClientInfo() = delete;

    ClientInfo(TCPSocket &&socket_, const String &name_, const String &address_)
    : socket{std::move(socket_)}
    , name{name_}
    , address{address_} {}
};

enum class Status {
    CONNECTED,
    DISCONNECTED
};

// TODO: ditch this
struct Disconnected {};

inline void send_message(const TCPSocket &socket, const ServerMessage &message) {
    std::array<std::byte, 1024> buffer{};
    SimpleInserter inserter{std::span{buffer.begin(), buffer.end()}};
    Serializer<ServerMessage>::serialize(message, inserter);
    socket.send(std::span{buffer.begin(), buffer.begin() + inserter.index});
}

inline AcceptedPlayer get_accepted_player(const ClientInfo &info) {
    AcceptedPlayer result{};
    result.GET_FIELD(player).GET_FIELD(name) = info.name;
    result.GET_FIELD(player).GET_FIELD(address) = info.address;
    return result;
}

inline bool is_complete_message(std::span<std::byte> span) {
    if (!span.size_bytes())
        return false;
    
    auto it = span.begin();
    switch (*it) {
        case Join::ID:
            if (span.size_bytes() > 1)
                return span.size_bytes() >= std::to_integer<std::size_t>(*++it) + 2;
            return false;
        case PlaceBlock::ID:
        case PlaceBomb::ID:
            return true;
        case Move::ID:
            return span.size_bytes() > 1;
        default:
            throw std::runtime_error{"TODO"}; // TODO
    }
}

Status player_routine(const std::size_t game_length, const std::size_t players_count,
                      const Monitor<std::vector<ClientInfo>> &players, const ClientInfo &info,
                      Monitor<std::optional<ClientMessage>> &reply,
                      const Monitor<std::vector<ServerMessage>> &server_messages)
{
    try {
        std::size_t player_index = 0;
        while (player_index < players_count) {
            const std::size_t current_players_count = players.lock().get().size();
            while (player_index < current_players_count)
                send_message(
                    info.socket,
                    get_accepted_player(players.lock().get().at(player_index++))
                );
            std::this_thread::yield();
        }

        GameStarted game_started{};
        for (std::size_t i = 0; i < players_count; ++i)
            game_started.GET_FIELD(players).insert({
                static_cast<PlayerId>(i), [&]() {
                    Player player{};
                    auto lock = players.lock();
                    player.GET_FIELD(name) = lock.get().at(i).name;
                    player.GET_FIELD(address) = lock.get().at(i).address;
                    return player;
                }()
            });
        send_message(info.socket, game_started);

        std::array<std::byte, 512> buffer{};
        auto begin_it = buffer.begin();
        auto end_it = buffer.begin();
        std::size_t message_index = 0;

        while (message_index <= game_length) {
            if (is_complete_message(std::span<std::byte>{begin_it, end_it})) {
                SimpleConsumer consumer{std::span<std::byte>{begin_it, end_it}};
                auto client_message = Serializer<ClientMessage>::deserialize(consumer);
                reply.lock().get() = std::move(client_message);
                begin_it += consumer.index;
            }

            if (end_it == buffer.end() && begin_it != buffer.begin()) {
                const std::size_t distance = static_cast<std::size_t>(std::distance(begin_it, end_it));
                std::memmove(
                    reinterpret_cast<void*>(buffer.data()),
                    reinterpret_cast<const void*>(&*begin_it),
                    distance
                );
                begin_it = buffer.begin();
                end_it = begin_it + distance;
            }

            if (end_it != buffer.end())
                info.socket.receive(std::span<std::byte>{end_it, buffer.end()});

            /* lock */ {
                /* TODO: exception handling */
                auto lock = server_messages.lock();
                if (message_index < lock.get().size())
                    send_message(info.socket, lock.get().at(message_index));
                ++message_index;
            }
        }
    } catch (const Disconnected&) {
        return Status::DISCONNECTED;
    }

    return Status::CONNECTED;
}

Status observer_routine(const std::size_t game_length, const std::size_t players_count,
                        const std::vector<ClientInfo> &players, const ClientInfo &info,
                        const Monitor<std::vector<ServerMessage>> &server_messages)
{
    try {
        std::size_t player_index = 0;
        while (player_index < players_count) {
            const std::size_t current_players_count = players.size();
            while (player_index < current_players_count)
                send_message(
                    info.socket,
                    get_accepted_player(players[player_index++])
                );
        }

        GameStarted game_started{};
        for (std::size_t i = 0; i < players_count; ++i)
            game_started.GET_FIELD(players).insert({
                static_cast<PlayerId>(i), [&]() {
                    Player player{};
                    player.GET_FIELD(name) = players[i].name;
                    player.GET_FIELD(address) = players[i].address;
                    return player;
                }()
            });
        send_message(info.socket, game_started);
        
        std::size_t message_index = 0;
        while (message_index <= game_length) {
            /* TODO: exception handling */
            auto lock = server_messages.lock();
            if (message_index < lock.get().size())
                send_message(info.socket, lock.get().at(message_index));
            ++message_index;
        }
    } catch (const Disconnected&) {
        return Status::DISCONNECTED;
    }
    
    return Status::CONNECTED;
}

class Messenger {
private:
    struct GameState {
        Monitor<std::vector<ServerMessage>> server_messages;
        Monitor<std::vector<ClientInfo>> players;
        std::vector<Monitor<std::optional<ClientMessage>>> player_messages;
        std::vector<ClientInfo> observers{};
        std::vector<std::future<Status>> tasks{};

        GameState(const std::size_t game_length, const std::size_t players_count)
        : server_messages(game_length)
        , players(players_count)
        , player_messages(players_count) {}
    };

private:
    const std::size_t game_length;
    const std::size_t players_count;
    
    ThreadPool thread_pool;
    GameState game_state;

public:
    Messenger() = delete;
    Messenger(std::size_t game_length_, std::size_t players_count_,
              std::size_t thread_count_ = std::thread::hardware_concurrency())
    : game_length{game_length_}
    , players_count{players_count_}
    , thread_pool{thread_count_}
    , game_state{game_length, players_count} {}

    void add_player(ClientInfo &&info) {
        auto lock = game_state.players.lock();
        const std::size_t index = lock.get().size();
        lock.get().push_back(std::move(info));
        game_state.player_messages.emplace_back(Monitor<std::optional<ClientMessage>>{std::nullopt});
        game_state.tasks.push_back(thread_pool.add_task(
            player_routine,
            game_length,
            players_count,
            std::ref(game_state.players),
            std::ref(lock.get()[index]),
            std::ref(game_state.player_messages[index]),
            std::ref(game_state.server_messages)
        ));
    }

    void add_observer(ClientInfo &&info) {
        const std::size_t index = game_state.observers.size();
        game_state.observers.push_back(std::move(info));
        game_state.tasks.push_back(thread_pool.add_task(
            observer_routine,
            game_length,
            players_count,
            std::ref(game_state.players.lock().get()),
            std::ref(game_state.observers[index]),
            std::ref(game_state.server_messages.lock().get())
        ));
    }

    void send_message(const ServerMessage &message) {
        game_state.server_messages.lock().get().push_back(message);
    }

    void send_message(ServerMessage &&message) {
        game_state.server_messages.lock().get().push_back(std::move(message));
    }

    std::vector<std::optional<ClientMessage>> get_messages() const {
        std::vector<std::optional<ClientMessage>> result{};
        for (const auto &message : game_state.player_messages)
            result.push_back(message.lock().get());
        return result;
    }

    std::vector<ClientInfo> clear() {
        std::vector<ClientInfo> result{};
        
        for (std::size_t i = 0; i < players_count; ++i)
            if (game_state.tasks[i].get() == Status::CONNECTED)
                result.push_back(std::move(game_state.players.lock().get()[i]));

        for (std::size_t i = 0; i < game_state.observers.size(); ++i)
            if (game_state.tasks[i + players_count].get() == Status::CONNECTED)
                result.push_back(std::move(game_state.observers[i]));
        
        // game_state = GameState{game_length, players_count}; // TODO

        return result;
    }
};

} // namespace SK

#endif // __SK_MESSENGER_H__
