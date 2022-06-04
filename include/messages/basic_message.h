#ifndef __SK_MESSAGES_BASIC_MESSAGE_H__
#define __SK_MESSAGES_BASIC_MESSAGE_H__

#include <messages/serializer.h>
#include <utilities/smart_struct.h>

#include <optional>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace SK {

/* Basic message -- a general wrapper for messages */
template<typename...>
struct BasicMessage;

template<typename... Types, ConstexprString... Names>
struct BasicMessage<Field<Types, Names>...> : public SmartStruct<Field<Types, Names>...> {
private:
    using Super = SmartStruct<Field<Types, Names>...>;

public:
    using Type = BasicMessage<Field<Types, Names>...>;

    template<typename F>
        requires std::is_invocable_v<F, Types&...>
    void apply(F &&f) {
        std::apply(std::move(f), Super::values);
    }

    template<typename F>
        requires std::is_invocable_v<F, const Types&...>
    void apply(F &&f) const {
        std::apply(std::move(f), Super::values);
    }
};

template<typename... Types, ConstexprString... Names>
class Serializer<BasicMessage<Field<Types, Names>...>> final {
public:
    using Self = BasicMessage<Field<Types, Names>...>;

    template<IsInserter Inserter>
    static void serialize(const Self &self, Inserter &inserter) {
        self.apply([&](const Types &...fields) {
            Serializer<Self>::serialize_values<Inserter, Types...>(inserter, fields...);
        });
    }

    template<IsConsumer Consumer>
    static Self deserialize(Consumer &consumer) {
        Self result{};
        result.apply([&](Types &...fields) {
            Serializer<Self>::assign_values<Consumer, Types...>(consumer, fields...);
        });
        return result;
    }

private:
    template<IsInserter Inserter>
    static void serialize_values([[maybe_unused]] Inserter &inserter) {}

    template<IsInserter Inserter, typename T, typename... Ts>
    static void serialize_values(Inserter &inserter, const T &field, const Ts &...rest) {
        Serializer<T>::serialize(field, inserter);
        serialize_values(inserter, rest...);
    }

    template<IsConsumer Consumer>
    static void assign_values([[maybe_unused]] Consumer &consumer) {}

    template<IsConsumer Consumer, typename T, typename... Ts>
    static void assign_values(Consumer &consumer, T &field, Ts &...rest) {
        field = Serializer<T>::deserialize(consumer);
        assign_values(consumer, rest...);
    }
};

/*
    Message -- a general wrapper for messages with an ID.
    The idea for this struct is purely to give the user
    a convenient way to define classes of messages
    identified by an ID. Then that class of messages
    can be represented by an std::variant consisting
    of those types.
*/
template<std::size_t Id, typename... Types>
struct Message : public BasicMessage<Types...> {
    using Super = BasicMessage<Types...>;
    static constexpr std::byte ID = std::byte{Id};

    Message() = default;
    Message(Super &&base)
    : Super{std::move(base)} {}
    Message(const Super &base)
    : Super{base} {}
};

/* Variant of basic messages -- implementation of serialisation and deserialisation */
namespace detail {

template<typename>
struct is_message_t : public std::false_type {};

template<std::size_t Id, typename... Types>
struct is_message_t<Message<Id, Types...>> : public std::true_type {};

} // namespace detail

template<typename T>
concept IsMessage = detail::is_message_t<T>::value;

template<typename... MessageTypes>
    requires (IsMessage<MessageTypes> && ...)
class Serializer<std::variant<MessageTypes...>> final {
public:
    using Self = std::variant<MessageTypes...>;

    template<IsInserter Inserter>
    static void serialize(const Self &self, Inserter &inserter) {
        return std::visit([&](const auto &variant) {
            using T = std::decay_t<decltype(variant)>;
            return Serializer<T>::serialize(variant, inserter);
        }, self);
    }

    template<IsConsumer Consumer>
    static Self deserialize(Consumer &consumer) {
        // A dummy instance of Message to get the type of the field ID.
        using id_t = decltype(Message<0>::ID);
        
        std::optional<Self> maybe_result = std::nullopt;
        const id_t id = consumer.get();
        consumer.pop();

        auto loop = [&]<typename T>() {
            if (T::ID == id)
                maybe_result = std::optional<Self>(Serializer<T>::deserialize(consumer));
        };

        (loop.template operator()<MessageTypes>(), ...);

        if (maybe_result)
            return std::move(maybe_result.get());
        else
            throw std::runtime_error{
                "[Message: deserialize] The ID of the message does not match any known one."
            };
    }
};

} // namespace SK

#endif // __SK_MESSAGES_BASIC_MESSAGE_H__
