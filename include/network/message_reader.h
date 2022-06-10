#ifndef __SK_NETWORK_MESSAGE_READER_H__
#define __SK_NETWORK_MESSAGE_READER_H__

#include <messages/client_messages.h>
#include <messages/server_messages.h>
#include <network/network_reader.h>

#include <optional>

namespace SK {

class MessageReader : public NetworkReader {
private:
    constexpr static std::size_t DEFAULT_SIZE = 1024;
    // Mutable: the size of the buffer may change
    // when there occurs an attempt to send a message
    // that does not fit in it.
    mutable std::vector<std::byte> buffer;

public:
    template<typename... Args>
    MessageReader(Args &&...args)
    : NetworkReader(std::forward<Args>(args)...)
    , buffer(DEFAULT_SIZE) {}

    bool stores_complete_message() const;
    std::optional<ClientMessage> fetch_message();
    void send_message(const ServerMessage &message) const;
};

} // namespace SK

#endif // __SK_NETWORK_MESSAGE_READER_H__
