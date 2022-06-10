#include <network/socket_options.h>
#include <utilities/miscellaneous.h>
#include <network/socket.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <bit>  // std::endian
#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace SK {

TCPSocket::TCPSocket() {
    socket_fd = ::socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1)
        throw std::runtime_error{strerror(errno)}; // TODO
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

void TCPSocket::set_socket_blocking(bool value) {
    if (socket_fd == -1)
        return;
    int flags = ::fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1)
        return;
    flags = value ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    if (::fcntl(socket_fd, F_SETFL, flags) != 0)
        throw std::runtime_error{strerror(errno)}; // TODO
}

namespace {

inline decltype(auto) get_option_value(const auto &option) {
    using type          = std::decay_t<decltype(option)>;
    using value_type    = typename type::value_type;

    if constexpr (std::same_as<value_type, bool>) {
        return static_cast<int>(option.value);
    } else if constexpr (std::same_as<type, ReceiveTimeout>) {
        timeval result;
        result.tv_sec = option.value / 1000;
        result.tv_usec = (option.value % 1000) * 1000;
        return result;
    } else {
        return option.value;
    }
}

} // anonymous namespace

TCPSocket &TCPSocket::set_socket_option(const SocketOption &option) {
    auto set_option = [&](const auto &value, auto level, auto optname) {
        return ::setsockopt(
            socket_fd,
            to_underlying(level),
            to_underlying(optname),
            reinterpret_cast<const void*>(&value),
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
            const auto level    = type::option_level;
            const auto optname  = type::option_type;

            return set_option(get_option_value(value), level, optname);
        },
        option
    ));

    return *this;
}

void TCPSocket::bind(std::uint16_t port) {
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_addr   = in6addr_any;
    address.sin6_port   = [&]() {
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

std::optional<TCPSocket> TCPSocket::accept() {
    int sock_fd = ::accept(socket_fd, nullptr, nullptr);
    if (sock_fd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return std::nullopt;
        throw std::runtime_error{strerror(errno)}; // TODO
    }
    return TCPSocket{sock_fd};
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
