#ifndef __SK_NETWORK_MESSAGE_READER_H__
#define __SK_NETWORK_MESSAGE_READER_H__

#include <messages/client_messages.h>
#include <network/network_reader.h>

#include <optional>

namespace SK {

class MessageReader : protected NetworkReader {
public:
    template<typename... Args>
    MessageReader(Args &&...args)
    : NetworkReader(std::forward<Args>(args)...) {}

    bool stores_complete_message() const;
    std::optional<ClientMessage> fetch_message();
};

} // namespace SK

#endif // __SK_NETWORK_MESSAGE_READER_H__
