#include <network/message_reader.h>

#include <cstddef>  // std::byte
#include <iterator> // std::next
#include <stdexcept>

namespace SK {

namespace {

struct SimpleConsumer {
    std::span<std::byte> span;
    std::size_t index = 0;

    using value_type = std::byte;

    SimpleConsumer(std::span<std::byte> span_)
    : span{span_} {}

    std::byte get() const {
        return span[index];
    }

    void pop() {
        ++index;
    }
};

} // namespace

bool MessageReader::stores_complete_message() const {
    const std::size_t distance = static_cast<std::size_t>(std::distance(begin, end));
    if (!distance)
        return false;
        
    auto it = begin;
    switch (*it) {
        case Join::ID:
            if (distance > 1)
                return distance >= std::to_integer<std::size_t>(*++it) + 2;
            return false;
        case PlaceBlock::ID:
        case PlaceBomb::ID:
            return true;
        case Move::ID:
            return distance > 1;
        default:
            throw std::runtime_error{"TODO"}; // TODO
    }
}

std::optional<ClientMessage> MessageReader::fetch_message() {
    if (!stores_complete_message())
        return std::nullopt;
        
    SimpleConsumer consumer{{begin, end}};
    ClientMessage result = Serializer<ClientMessage>::deserialize(consumer);
    begin = std::next(begin, static_cast<typename iter_type::difference_type>(consumer.index));

    return result;
}

namespace {

struct VectorInserter {
    std::vector<std::byte> &vector;
    std::size_t index = 0;

    using value_type = std::byte;

    void push(std::byte byte) {
        if (index < vector.size())
            vector[index] = byte;
        else
            vector.push_back(byte);
        ++index;
    }
};

} // namespace

void MessageReader::send_message(const ServerMessage &message) const {
    VectorInserter inserter{.vector = buffer};
    Serializer<ServerMessage>::serialize(message, inserter);
    using diff_type = typename decltype(buffer)::iterator::difference_type;
    NetworkReader::send({
        buffer.begin(),
        std::next(buffer.begin(), static_cast<diff_type>(inserter.index))
    });
}

} // namespace SK
