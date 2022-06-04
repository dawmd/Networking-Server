#ifndef __SK_MESSAGES_MAP_H__
#define __SK_MESSAGES_MAP_H__

#include <messages/common.h>
#include <messages/serializer.h>

#include <map>

namespace SK {

template<typename K, typename V>
    requires Serializable<K> && Serializable<V>
class Map : public std::map<K, V> {
public:
    using SizeType = u32;

    template<IsInserter Inserter>
    void serialize(Inserter &inserter) const {
        const SizeType size = static_cast<SizeType>(this->size());
        Serializer<SizeType>::serialize(size, inserter);

        SizeType counter = 0;
        for (const auto &[key, value] : *this) {
            if (counter++ == size)
                break;
            
            Serializer<K>::serialize(key, inserter);
            Serializer<V>::serialize(value, inserter);
        }
    }
};

template<typename K, typename V>
class Serializer<Map<K, V>> final {
public:
    template<IsInserter Inserter>
    static void serialize(const Map<K, V> &map, Inserter &inserter) {
        map.serialize(inserter);
    }

    template<IsConsumer Consumer>
    static Map<K, V> deserialize(Consumer &consumer) {
        using SizeType = typename Map<K, V>::SizeType;
        const SizeType size = Serializer<SizeType>::deserialize(consumer);
        Map<K, V> result{};

        for (SizeType i = 0; i < size; ++i) {
            K key = Serializer<K>::deserialize(consumer);
            V value = Serializer<V>::deserialize(consumer);
            result.insert({std::move(key), std::move(value)});
        }

        return result;
    }
};

} // namespace SK

#endif // __SK_MESSAGES_MAP_H__
