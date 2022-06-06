#ifndef __SK_NETWORK_SOCKET_H__
#define __SK_NETWORK_SOCKET_H__

#include <network/socket_options.h>

#include <cstdint>
#include <span>

namespace SK {
class TCPSocket {
private:
    int socket_fd = -1;

public:
    TCPSocket();

    TCPSocket(TCPSocket&&);
    TCPSocket &operator=(TCPSocket&&);

    TCPSocket(const TCPSocket&) = delete;
    TCPSocket &operator=(const TCPSocket&) = delete;

    ~TCPSocket();

    TCPSocket &set_socket_option(const SocketOption &option);

    void bind(std::uint16_t port);
    void listen(int queue_length);
    TCPSocket accept();
    // void connect(); // not needed

    std::size_t receive(std::span<std::byte> span) const;
    void send(std::span<std::byte> span) const;
};

} // namespace SK

#endif // __SK_NETWORK_SOCKET_H__
