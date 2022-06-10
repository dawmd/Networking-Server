#include "network/socket.h"
#include <network/network_reader.h>

#include <cstring>  // std::memmove

namespace SK {

NetworkReader::NetworkReader(TCPSocket &&socket_, std::size_t buffer_size_)
: socket{std::move(socket_)}
, buffer(buffer_size_)
, begin{buffer.begin()}
, end{buffer.begin()} {} // TODO: set up the socket, options, etc.

std::size_t NetworkReader::receive() {
    if (end == buffer.end()) {
        if (begin == buffer.begin())
            return 0;
            
        auto distance = std::distance(buffer.begin(), begin);
        std::memmove(
            buffer.data(),
            std::addressof(*begin),
            static_cast<std::size_t>(distance)
        );
        begin = buffer.begin();
        end = begin + distance;
    }

    return socket.receive({end, buffer.end()});
}

} // namespace SK
