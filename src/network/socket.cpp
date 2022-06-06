#include <utilities/miscellaneous.h>
#include <network/socket.h>

#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <type_traits>
#include <unistd.h>

#include <bit>  // std::endian

#include <iostream>
#include <cstring>

namespace SK {

TCPSocket::TCPSocket() {
    std::cout << "b\n";
    socket_fd = ::socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    std::cout << "b\n";
    if (socket_fd == -1)
        throw std::runtime_error{strerror(errno)}; // TODO
    std::cout << "b\n";
}

TCPSocket::TCPSocket(TCPSocket &&other) {
    if (socket_fd != -1)
        close(socket_fd);
    socket_fd = other.socket_fd;
    other.socket_fd = -1;
}

TCPSocket &TCPSocket::operator=(TCPSocket &&other) {
    if (socket_fd != -1)
        close(socket_fd);
    socket_fd = other.socket_fd;
    other.socket_fd = -1;
    return *this;
}

TCPSocket::~TCPSocket() {
    if (socket_fd != -1) {
        close(socket_fd);
        socket_fd = -1;
    }
}

TCPSocket &TCPSocket::set_socket_option(const SocketOption &option) {
    auto set_option = [&](const auto &value, auto level, auto optname) {
        return ::setsockopt(
            socket_fd,
            to_underlying(level),
            to_underlying(optname),
            reinterpret_cast<const void*>(value),
            static_cast<socklen_t>(sizeof(value))
        );
    };

    auto resolve_error = [](int error) {
        if (error == -1)
            throw std::runtime_error{strerror(errno)}; // TODO
    };

    resolve_error(std::visit(
        [&](const auto &value) {
            using type          = std::decay_t<decltype(value)>;
            using value_type    = typename type::value_type;
            const auto level    = type::option_level;
            const auto optname  = type::option_type;

            if constexpr (std::same_as<value_type, bool>)
                return set_option(static_cast<int>(value.value), level, optname);
            else
                return set_option(value.value, level, optname);
        },
        option
    ));

    return *this;
}

void TCPSocket::bind(std::uint16_t port) {
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = [&]() {
        if constexpr (std::endian::native == std::endian::big)
            return port;
        else
            return swap_endiannes(port);
    }();

    auto result = ::bind(
        socket_fd,
        reinterpret_cast<const sockaddr*>(&address),
        static_cast<socklen_t>(sizeof(address))
    );

    if (result == -1)
        throw std::runtime_error{"TODO"}; // TODO
}

void TCPSocket::listen(int queue_length) {
    if (::listen(socket_fd, queue_length) == -1)
        throw std::runtime_error{"TODO"}; // TODO
}

TCPSocket TCPSocket::accept() {
    TCPSocket result{};
    result.socket_fd = ::accept(socket_fd, nullptr, nullptr);
    if (result.socket_fd == -1)
        throw std::runtime_error{strerror(errno)}; // TODO
    return result;
}

std::size_t TCPSocket::receive(std::span<std::byte> span) const {
    /* TODO: flags */
    ssize_t result = ::recv(socket_fd, reinterpret_cast<void*>(span.data()), span.size_bytes(), 0);
    if (result == -1)
        throw std::runtime_error{"TODO"}; // TODO
    return static_cast<std::size_t>(result);
}

void TCPSocket::send(std::span<std::byte> span) const {
    /* TODO: flags */
    ssize_t sent_size = ::send(socket_fd, reinterpret_cast<const void*>(span.data()), span.size_bytes(), 0);
    if (sent_size == -1 || static_cast<std::size_t>(sent_size) != span.size_bytes())
        throw std::runtime_error{"TODO"}; // TODO
}

} // namespace SK
