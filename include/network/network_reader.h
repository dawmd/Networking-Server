#ifndef __SK_NETWORK_NETWORK_READER_H__
#define __SK_NETWORK_NETWORK_READER_H__

#include <network/socket.h>

#include <cstddef>  // std::byte
#include <span>
#include <vector>

namespace SK {

class NetworkReader {
private:
    TCPSocket socket;
    std::vector<std::byte> buffer;

protected:
    using iter_type = typename decltype(buffer)::iterator;

    iter_type begin;
    iter_type end;

public:
    NetworkReader() = delete;
    
    NetworkReader(TCPSocket &&socket_, const std::size_t buffer_size_ = 512);
    
    NetworkReader(NetworkReader&&) = default;
    NetworkReader &operator=(NetworkReader&&) = default;
    
    NetworkReader(const NetworkReader&) = delete;
    NetworkReader &operator=(const NetworkReader&) = delete;

    const std::span<std::byte> span() const noexcept {
        return {begin, end};
    }
    std::size_t receive();
};

} // namespace SK

#endif // __SK_NETWORK_NETWORK_READER_H__
