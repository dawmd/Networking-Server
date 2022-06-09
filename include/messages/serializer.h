#ifndef __SK_MESSAGES_SERIALIZER_H__
#define __SK_MESSAGES_SERIALIZER_H__

#include <utilities/miscellaneous.h>

#include <bit>          // std::endian
#include <concepts>
#include <cstddef>

namespace SK {

template<typename>
class Serializer;

template<typename T, typename U = std::byte>
concept IsConsumer = requires (T &consumer) {
    { consumer.get() } -> std::convertible_to<U>;
    consumer.pop();
};

template<typename T, typename U = typename T::value_type>
concept IsInserter = requires (T &inserter, const U &value) {
    inserter.push(value);
};

namespace detail {

// A dummy type used for defining the concept Serializable below.
template<typename T>
struct PhantomConsumer {
    T get();
    void pop();
};

// A dummy type for defining the concept Serializable below.
struct PhantomInserter {
    using value_type = int;
    void push(int);
};

} // namespace detail

template<
    typename T,
    typename Inserter = detail::PhantomInserter,
    typename Consumer = detail::PhantomConsumer<std::byte>
>
concept Serializable = requires (T &t, Inserter &inserter, Consumer &consumer) {
    Serializer<T>::serialize(t, inserter);
    { Serializer<T>::deserialize(consumer) } -> std::same_as<T>;
};

/* Serializer for numeric types and std::byte */
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

        Type result = *reinterpret_cast<Type*>(bytes);

        // We need to swap the bytes because the endianness
        // of the network is big.
        if constexpr (std::endian::native == std::endian::little)
            return swap_endiannes(result);
        else
            return result;
    }
};

} // namespace SK

#endif // __SK_MESSAGES_SERIALIZER_H__
