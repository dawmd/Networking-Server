#ifndef __SK_MESSAGES_SERIALIZER_H__
#define __SK_MESSAGES_SERIALIZER_H__

#include <utilities/byte_inserter.h>
#include <utilities/miscellaneous.h>

#include <algorithm>    // std::reverse
#include <bit>          // std::endian
#include <concepts>
#include <cstddef>

namespace SK {

template<typename T, typename U = std::byte>
concept IsConsumer = requires (T &consumer) {
    { consumer.get() } -> std::convertible_to<U>;
    consumer.pop();
};

template<typename T, typename U = typename T::value_type>
concept IsInserter = requires (T &inserter, const U &value) {
    inserter.push(value);
};

template<typename>
class Serializer;

template<typename T, typename Inserter = ByteQueue>
concept Serializable = requires (const T &t, Inserter &queue) {
    // ByteQueue satisfies both IsConsumer and IsInserter concepts.
    // The choice here was made arbitrarily just so we can only
    // have one template parameter for the concept.
    Serializer<T>::serialize(t, queue);
    { Serializer<T>::deserialize(queue) } -> std::same_as<T>;
};

template<typename T>
    requires std::integral<T> ||
             std::floating_point<T> ||
             std::same_as<std::decay_t<T>, std::byte>
class Serializer<T> final {
public:
    using Type = std::decay_t<T>;

    template<typename Inserter>
        requires IsInserter<Inserter>
    static void serialize(Type number, Inserter &inserter) {
        // We need to swap the bytes because the endianness
        // of the network is big.
        if constexpr (std::endian::native == std::endian::little)
            number = swap_endiannes(number);

        std::byte *bytes = reinterpret_cast<std::byte*>(&number);
        for (std::size_t i = 0; i < sizeof(number); ++i)
            inserter.push(bytes[i]);
    }

    template<typename Consumer>
        requires IsConsumer<Consumer, std::byte>
    static Type deserialize(Consumer &consumer) {
        constexpr std::size_t size = sizeof(Type);
        std::byte bytes[size];

        for (std::size_t i = 0; i < size; ++i) {
            bytes[i] = consumer.get();
            consumer.pop();
        }

        // We need to swap the bytes because the endianness
        // of the network is big.
        if constexpr (std::endian::native == std::endian::little)
            std::reverse(bytes, bytes + size);
        
        return *reinterpret_cast<Type*>(bytes);
    }
};

} // namespace SK

#endif // __SK_MESSAGES_SERIALIZER_H__
