#ifndef __SK_MESSAGES_STRING_H__
#define __SK_MESSAGES_STRING_H__

#include <messages/common.h>
#include <messages/serializer.h>

#include <string>

namespace SK {

class String final : public std::string {
public:
    using SizeType = u8;
    using CharType = typename std::string::value_type;

    template<IsInserter Inserter>
    void serialize(Inserter &inserter) const {
        const SizeType size = static_cast<SizeType>(this->size());
        Serializer<SizeType>::serialize(size, inserter);
        
        for (SizeType i = 0; i < size; ++i)
            Serializer<CharType>::serialize((*this)[i], inserter);
    }
};

template<>
class Serializer<String> final {
public:
    template<IsInserter Inserter>
    static void serialize(const String &string, Inserter &inserter) {
        string.serialize(inserter);
    }

    template<IsConsumer Consumer>
    static String deserialize(Consumer &consumer) {
        using SizeType = typename String::SizeType;
        using CharType = typename String::CharType;

        const SizeType size = Serializer<SizeType>::deserialize(consumer);
        String result{};
        result.resize(size);

        for (SizeType i = 0; i < size; ++i)
            result[i] = Serializer<CharType>::deserialize(consumer);
        return result;
    }
};

} // namespace SK

#endif // __SK_MESSAGES_STRING_H__
