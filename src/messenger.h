#ifndef __SK_MESSENGER_H__
#define __SK_MESSENGER_H__

#include <messages/client_messages.h>
#include <messages/network_string.h>
#include <messages/server_messages.h>
#include <network/socket.h>
#include <utilities/monitor.h>
#include <utilities/thread_pool.h>

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

namespace SK {

struct ClientInfo {
    TCPSocket socket;
    String name;
    String address;
};

enum class Status {
    CONNECTED,
    DISCONNECTED
};

Status player_routine(const std::size_t game_length, const std::size_t players_count,
                      const Monitor<std::vector<ClientInfo>> &players, const ClientInfo &info,
                      Monitor<std::optional<ClientMessage>> &reply)
{
    
}

Status observer_routine(const std::size_t game_length, const std::size_t players_count,
                        const std::vector<ClientInfo> &players, const ClientInfo &info)
{

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
        game_state.player_messages.push_back(std::nullopt);
        game_state.tasks.push_back(thread_pool.add_task(
            player_routine,
            game_length,
            players_count,
            std::ref(game_state.players),
            std::ref(lock.get()[index]),
            std::ref(game_state.player_messages[index])
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
            std::ref(game_state.observers[index])
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
        
        game_state = GameState{game_length, players_count};

        return result;
    }
};

} // namespace SK

#endif // __SK_MESSENGER_H__
