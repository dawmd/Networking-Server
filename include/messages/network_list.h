#ifndef __SK_MESSAGES_LIST_H__
#define __SK_MESSAGES_LIST_H__

#include <messages/common.h>
#include <messages/serializer.h>

#include <vector>

namespace SK {

template<typename T>
    requires Serializable<T>
class List : public std::vector<T> {
public:
    using SizeType = u32;

    template<typename... Args>
    List(Args &&...args)
    : std::vector<T>(std::forward<Args>(args)...) {}

    template<IsInserter Inserter>
    void serialize(Inserter &inserter) const {
        const SizeType size = static_cast<SizeType>(this->size());
        Serializer<SizeType>::serialize(size, inserter);

        for (SizeType i = 0; i < size; ++i)
            Serializer<T>::serialize((*this)[i], inserter);
    }
};

template<typename T>
class Serializer<List<T>> final {
public:
    using Type = List<T>;

    template<IsInserter Inserter>
    static void serialize(const List<T> &list, Inserter &inserter) {
        return list.serialize(inserter);
    }

    template<IsConsumer Consumer>
    static List<T> deserialize(Consumer &consumer) {
        using SizeType = typename Type::SizeType;
        List<T> result{};
        const SizeType size = Serializer<SizeType>::deserialize(consumer);
        for (SizeType i = 0; i < size; ++i)
            result.push_back(Serializer<T>::deserialize(consumer));
        return result;
    }
};

} // namespace SK

#endif // __SK_MESSAGES_LIST_H__
