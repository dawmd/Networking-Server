#ifndef __SK_NETWORK_SOCKET_OPTIONS_H__
#define __SK_NETWORK_SOCKET_OPTIONS_H__

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <variant>

namespace SK {

namespace detail {

enum class SocketOptionType : int {
    REUSE_PORT  = SO_REUSEPORT,
    RCV_TIMEOUT = SO_RCVTIMEO,
    IPV6_ONLY   = IPV6_V6ONLY
};

enum class SocketOptionLevel : int {
    SOCKET      = SOL_SOCKET,
    IPV6_PROTO  = IPPROTO_IPV6
};

template<SocketOptionType Type, SocketOptionLevel Level, typename Value>
struct SocketOption {
    constexpr static SocketOptionType option_type = Type;
    constexpr static SocketOptionLevel option_level = Level;

    using value_type = Value;

    Value value;

    SocketOption() = delete;
    SocketOption(const Value &option_value) : value{option_value} {}
};

} // namespace detail

using ReusePort = detail::SocketOption<
    detail::SocketOptionType::REUSE_PORT,
    detail::SocketOptionLevel::SOCKET,
    bool
>;

using ReceiveTimeout = detail::SocketOption<
    detail::SocketOptionType::RCV_TIMEOUT,
    detail::SocketOptionLevel::SOCKET,
    std::size_t // in miliseconds
>;

using IPv6Only = detail::SocketOption<
    detail::SocketOptionType::IPV6_ONLY,
    detail::SocketOptionLevel::IPV6_PROTO,
    bool
>;

using SocketOption = std::variant<ReusePort, ReceiveTimeout, IPv6Only>;

} // namespace SK

#endif // __SK_NETWORK_SOCKET_OPTIONS_H__
