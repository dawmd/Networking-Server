#include "messenger.h"

#include <thread>

#define GET_FIELD(name) get<#name>()

namespace SK {

// TODO: ditch these.
struct Disconnected {};
struct Timeout {};

Messenger::Messenger(std::size_t game_length_, std::size_t players_count_,
                     std::size_t thread_count_)
: game_length{game_length_}
, players_count{players_count_}
, thread_pool{thread_count_} {}

void Messenger::add_player(Client &&client, const PlayerInfo &info) {
    players.lock().get().emplace_back(info);
    statuses.push_back(thread_pool.add_task(
        &Messenger::player_routine,
        this,
        std::move(client)
    ));
}

void Messenger::add_observer(Client &&client) {
    statuses.push_back(thread_pool.add_task(
        &Messenger::observer_routine,
        this,
        std::move(client)
    ));
}

std::vector<Client> Messenger::finish() {
    std::vector<Client> result{};
    for (auto &future : statuses)
        if (future.get().status == ConnectionStatus::OK)
            result.push_back(std::move(future.get().client));
        
    reset();
    return result;
}

void Messenger::reset() {
    players.lock().get().clear();
    server_messages.lock().get().clear();
    statuses.clear();
}

void Messenger::send_accepted_player_messages(const MessageReader &reader) {
    std::size_t index = 0;
    while (index < players_count) {
        try {
            if (index < server_messages.lock().get().size()) {
                const auto &player = players.lock().get()[index];
                reader.send_message(AcceptedPlayer{player.name, player.address});
                ++index;
            } else {
                std::this_thread::yield();
            }
        } catch (const Timeout&) {
            // ...
        }
    }
}

void Messenger::send_game_started(const MessageReader &reader) {
    GameStarted message{};
    const auto &players_vec = players.lock().get();

    for (std::size_t i = 0; i < players_count; ++i) {
        try {
            message.GET_FIELD(players).insert({
                static_cast<PlayerId>(i),
                Player{players_vec[i].name, players_vec[i].address}
            });
        } catch (const Timeout&) {
            // ...
        }
    }

    reader.send_message(message);
}

Messenger::ClientStatus Messenger::player_routine(Client &&client) {
    try {
        send_accepted_player_messages(client.reader);
        send_game_started(client.reader);

        // Turns.
        std::size_t index = 0;
        while (index <= game_length) {
            client.reader.receive();
            if (client.reader.stores_complete_message())
                /* ??? = */ client.reader.fetch_message();
                
            if (index < server_messages.lock().get().size()) {
                try {
                    client.reader.send_message(server_messages.lock().get()[index]);
                    ++index;
                } catch (const Timeout&) {
                    // ...
                }
            } else {
                std::this_thread::yield();
            }
        }

        return {.client = std::move(client), .status = ConnectionStatus::OK};
    } catch (const Disconnected&) {
        return {.client = std::move(client), .status = ConnectionStatus::DISCONNECTED};
    }
}

Messenger::ClientStatus Messenger::observer_routine(Client &&client) {
    try {
        send_accepted_player_messages(client.reader);
        send_game_started(client.reader);

        // Turns.
        std::size_t index = 0;
        while (index <= game_length) {
            if (index < server_messages.lock().get().size()) {
                try {
                    client.reader.send_message(server_messages.lock().get()[index]);
                    ++index;
                } catch (const Timeout&) {
                    // ...
                }
            } else {
                std::this_thread::yield();
            }
        }

        return {.client = std::move(client), .status = ConnectionStatus::OK};
    } catch (const Disconnected&) {
        return {.client = std::move(client), .status = ConnectionStatus::DISCONNECTED};
    }
}

} // namespace SK
