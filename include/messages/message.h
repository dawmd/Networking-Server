#ifndef __SK_MESSAGES_MESSAGE_H__
#define __SK_MESSAGES_MESSAGE_H__

#include <messages/serializer.h>
#include <utilities/smart_struct.h>

#include <optional>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace SK {

/* Basic message -- a general wrapper for messages */
template<typename... Fields>
struct BasicMessage : public SmartStruct<Fields...> {
public:
    using Type = BasicMessage<Fields...>;

    template<typename F>
        requires std::is_invocable_v<F, typename Fields::Type&...>
    decltype(auto) apply(F &&f) {
        return std::apply(std::move(f), this->values);
    }

    template<typename F>
        requires std::is_invocable_v<F, const typename Fields::Type&...>
    decltype(auto) apply(F &&f) const {
        return std::apply(std::move(f), this->values);
    }
};

template<typename... Types, ConstevalString... Names>
class Serializer<BasicMessage<Field<Types, Names>...>> final {
public:
    using Self = BasicMessage<Field<Types, Names>...>;

    template<IsInserter Inserter>
    static void serialize(const Self &self, Inserter &inserter) {
        self.apply([&](const Types &...fields) {
            (Serializer<Types>::serialize(fields, inserter), ...);
        });
    }

    template<IsConsumer Consumer>
    static Self deserialize(Consumer &consumer) {
        Self result{};
        result.apply([&](Types &...fields) {
            ((fields = Serializer<Types>::deserialize(consumer)), ...);
        });
        return result;
    }
};

/*
    Message -- a general wrapper for messages with an ID.
    The idea for this struct is purely to give the user
    a convenient way to define a class of messages
    identified by IDs. Then, that class of messages
    can be represented by an std::variant consisting
    of those types.
*/
template<std::size_t Id, typename... Types>
struct Message : public BasicMessage<Types...> {
    using Super = BasicMessage<Types...>;
    using Type = Message<Id, Types...>;
    static constexpr std::byte ID = std::byte{Id};

    template<typename... Args>
    Message(Args &&...args)
    : Super(std::forward<Args>(args)...) {}
};

/* Variant of basic messages -- implementation of serialisation and deserialisation */
namespace detail {

template<typename>
struct is_message_t : public std::false_type {};

template<std::size_t Id, typename... Types>
struct is_message_t<Message<Id, Types...>> : public std::true_type {};

consteval bool no_id_duplicate(auto... ids) {
    std::size_t arr[] = { static_cast<std::size_t>(ids)... };
    std::sort(std::begin(arr), std::end(arr));
    const auto it_end = std::end(arr);
    return it_end == std::unique(std::begin(arr), std::end(arr));
}

} // namespace detail

template<typename T>
concept IsMessage = detail::is_message_t<T>::value;

template<typename... MessageTypes>
    requires (IsMessage<MessageTypes> && ...) &&
             (detail::no_id_duplicate((MessageTypes::ID)...))
using MessageWrapper = std::variant<MessageTypes...>;

template<typename... MessageTypes>
class Serializer<MessageWrapper<MessageTypes...>> final {
public:
    using Self = MessageWrapper<MessageTypes...>;

    template<IsInserter Inserter>
    static void serialize(const Self &self, Inserter &inserter) {
        std::visit([&](const auto &variant) {
            using T = std::decay_t<decltype(variant)>;
            using IdType = decltype(T::ID);

            Serializer<IdType>::serialize(T::ID, inserter);
            Serializer<typename T::Super>::serialize(variant, inserter);
        }, self);
    }

    template<IsConsumer Consumer>
    static Self deserialize(Consumer &consumer) {
        // A dummy instance of Message to get the type of the field ID.
        using IdType = decltype(Message<0>::ID);
        
        std::optional<Self> maybe_result = std::nullopt;
        
        const IdType id = consumer.get();
        consumer.pop();

        ([&]<typename T>() {
            using BasicMessageType = typename T::Super;
            using MessageType = typename T::Type;

            if (T::ID == id)
                maybe_result = static_cast<MessageType>(
                    Serializer<BasicMessageType>::deserialize(consumer)
                );
        }.template operator()<MessageTypes>(), ...);

        if (maybe_result)
            return std::move(maybe_result).value();
        else
            throw std::runtime_error{
                "[Message: deserialize] The ID of the message does not match any known one."
            };
    }
};

} // namespace SK

#endif // __SK_MESSAGES_MESSAGE_H__
