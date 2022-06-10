#ifndef __SK_MESSENGER_H__
#define __SK_MESSENGER_H__

#include <messages/network_string.h>
#include <messages/server_messages.h>
#include <network/message_reader.h>
#include <utilities/monitor.h>
#include <utilities/thread_pool.h>

#include <cstddef>  // std::size_t
#include <vector>

namespace SK {

struct Client {
    MessageReader reader;
    String address;
};

struct PlayerInfo {
    String name;
    String address;
};

class Messenger {
private:
    enum class ConnectionStatus {
        OK,
        DISCONNECTED
    };

    struct ClientStatus {
        Client client;
        ConnectionStatus status;
    };

private:
    const std::size_t game_length;
    const std::size_t players_count;

    ThreadPool thread_pool;

    Monitor<std::vector<PlayerInfo>> players{};

    Monitor<std::vector<ServerMessage>> server_messages{};
    std::vector<std::future<ClientStatus>> statuses{};

public:
    Messenger() = delete;

    Messenger(std::size_t game_length_, std::size_t players_count_,
              std::size_t thread_count_ = std::thread::hardware_concurrency());

    void add_player(Client &&client, const PlayerInfo &info);
    void add_observer(Client &&client);
    std::vector<Client> finish();

private:
    void reset();
    void send_accepted_player_messages(const MessageReader &reader);
    void send_game_started(const MessageReader &reader);
    ClientStatus player_routine(Client &&client);
    ClientStatus observer_routine(Client &&client);
};

} // namespace SK

#endif // __SK_MESSENGER_H__
