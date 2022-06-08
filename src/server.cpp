/*
 * Clients connected to the server at any moment 
 * of an ONGOING game are divided into three classes:
 * 1) players -- clients who are taking actual
 *    part in the game, move their robots, etc.,
 * 2) observers -- clients who are only receiving
 *    messages about the state of the game,
 * 3) passive clients -- clients who remain connected
 *    to the server, but who have not asked for a right
 *    to play. They might be players from
 *    a previous game.
 *
 * What happens during an ongoing game?
 *   The sockets corresponding to the players are stored in a vector,
 *   alongside information indicating whether they're still
 *   connected or not -- a disconnected client still technically
 *   takes part in the game, but no message is sent to or read from them.
 *
 *   The sockets corresponding to the observers are also stored in
 *   a vector. The only difference between them and the players is
 *   no message is attempted to be received from them. The reason
 *   for that is there is no point (from the perspective of performance)
 *   to waste time on making a syscall during the game, while handling
 *   the players efficiently is the main focus of the server.
 *   That is simply too expensive, especially considering a situation
 *   when such a client is notoriously sending big, yet still correct
 *   messages. It still can be detected when such a client disconnects,
 *   but whether they've sent an invalid message will only be learnt about
 *   afterwards.
 *
 *   The sockets corresponding to passive clients are ignored
 *   during the game. The server is trying neither reading from
 *   nor sending data over to them. However, this is not the case
 *   if the number of handled connections has not been reached yet.
 *   In a situation like that, another thread is going through
 *   the queue (check the section below to learn what the queue is),
 *   checking whether there are clients willing to join the game,
 *   and makes them observers. Do note here they have to receive
 *   information not only about the game having already been started,
 *   but also about every turn that has been taken in the game so far.
 *
 * What happens between games?
 *   When a game ends, the main thread (referred to by "the server" from
 *   now on in this section) is checking which players and observers have
 *   been disconnected and closes the corresponding sockets. The others
 *   are put into a queue where passive players are.
 *
 *   Then, the server takes one socket from the queue at a time, tries to
 *   fetch a message from it. If it does receive one, it parses it.
 *   If it's a Join message and there are still free slots in the game,
 *   it puts the socket into the vector of players. If it's another valid
 *   message, the server ignores it and put the socket back into the queue
 *   -- specifically at the end of it. If it's an invalid message,
 *   the server closes the socket.
 *
 *   As soon as the number of required players has been reached, the server
 *   starts the game. Only other threads will be taking care of clients
 *   who want to join the game -- making them observers.
 *
 *   When a new player joins the lobby, a message AcceptedPlayer has to be sent
 *   to everyone in the lobby, including the new player (who also receives every
 *   previous message so that they can learn who's waiting along them). An important
 *   note here is the messages about accepted players DO NOT have to be sent in the
 *   same order. It doesn't matter, at least from a player's perspective, if they
 *   joined the game as the first, or the second one. They just need to know who
 *   is in the lobby.
 *
 *   However, it is imperative all those messages have already been sent out when the server
 *   decides to start a game by notifying the players and observers by a message GameStarted.
 *   Violating this invariant might lead to a client deciding to disconnect, assuming the server
 *   is not working properly -- and that is reasonable.
 *
 * Responsibilities and tasks of threads
 *   The server is only taking care of updating the state of the game. It receives messages
 *   from the threads communicating with clients, and based on that, it simulates the game.
 *   Afterwards, it saves a message that's supposed to be sent to players and observers,
 *   notifies the threads they ought to send it, and waits for the next turn to end.
 *
 *   Threads managing players and observers work almost the same way. The only difference is
 *   those managin observers do not attempt to receive messages from the socket. This will
 *   be omitted in the analysis below for simplicity.
 *   The threads check if there are enough bytes in a buffer they store to parse them
 *   into a message. If so, they do and store it. Then they check if there has been a signal
 *   from the server they should hand over a message to it and if that's the case, they
 *   handle the request approprietly. Either way, then they attempt to read bytes from the client
 *   into the buffer. The sockets they use should have had timeout set to some value by the server
 *   so that the threads don't block and the value matches the game's conditions. Afterwards,
 *   they check if the server has sent a request to stop gathering data and send a message from it,
 *   or if the game has ended, in which he work is done.
 *
 * The listener
 *   There's a special thread that works along the server -- the listener. It accepts
 *   new connections and puts sockets corresponding to them into the queue of passive players.
 *
*/

#include <network/socket.h>
#include <network/socket_options.h>
#include <utilities/thread_pool.h>
#include <messages/client_messages.h>
#include <messages/server_messages.h>
#include <messages/serializer.h>

#include <array>
#include <atomic>
#include <cstring>  // std::memmove
#include <optional>
#include <span>
#include <queue>
#include <vector>

using namespace SK;

inline bool is_complete_message(std::span<std::byte> span) {
    if (!span.size_bytes())
        return false;
    
    auto it = span.begin();
    switch (*it) {
        case std::byte{0}:
            if (span.size_bytes() > 1)
                return span.size_bytes() == 2 + std::to_integer<std::size_t>(*++it);
            return false;
        case std::byte{1}:
        case std::byte{2}:
            return true;
        case std::byte{3}:
            return span.size_bytes() > 1;
        default:
            throw std::runtime_error{"TODO"}; // TODO
    }
}

std::optional<ClientMessage> messenger_routine(TCPSocket &socket, std::atomic_bool &should_finish) {
    std::optional<ClientMessage> result = std::nullopt;
    std::array<std::byte, 512> buffer{};
    auto begin = buffer.begin();
    auto end = buffer.end();

    struct ByteConsumer {
        std::span<std::byte> buffer;
        std::size_t index = 0;

        ByteConsumer(std::span<std::byte> span)
        : buffer{span} {}

        std::byte get() const {
            if (index < buffer.size_bytes())
                return buffer[index];
            throw std::out_of_range{"TODO"}; // TODO
        }

        void pop() {
            ++index;
        }
    };

    while (!should_finish) {
        if (end == buffer.end()) {
            const std::size_t byte_count = static_cast<std::size_t>(std::distance(begin, end));
            std::memmove(
                reinterpret_cast<void*>(buffer.data()),
                reinterpret_cast<const void*>(&*begin),
                byte_count
            );
            begin = buffer.begin();
            end = buffer.begin() + byte_count;
        }
        
        end += socket.receive(std::span{end, buffer.end()});
        
        if (is_complete_message(std::span{begin, end})) {
            ByteConsumer consumer{std::span{begin, end}};
            result = Serializer<ClientMessage>::deserialize(consumer);
            begin += consumer.index;
        }
    }

    return result;
}

void game(ThreadPool &thread_pool, TCPSocket &listener) {

}



void run() {
    ThreadPool thread_pool{};
    TCPSocket listener{};
    listener.set_socket_option(ReusePort{true});
    listener.bind(12345); // TODO
    listener.set_socket_option(IPv6Only{false});
    listener.listen(25); // TODO


}

int main() {
    run();
}
