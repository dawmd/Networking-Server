#ifndef __SK_UTILITIES_SMART_STRUCT_H__
#define __SK_UTILITIES_SMART_STRUCT_H__

#include <concepts>
#include <cstdlib>  // std::size_t
#include <tuple>
#include <type_traits>

namespace SK {

template<std::size_t length>
struct ConstexprString {
    char content[length];

    constexpr ConstexprString(const char (&characters)[length]) {
        for (std::size_t i = 0; i < length; ++i)
            content[i] = characters[i];
    }

    consteval bool operator==(const ConstexprString&) const = default;
};

template<std::size_t length_1, std::size_t length_2>
consteval bool operator==(const ConstexprString<length_1>&, const ConstexprString<length_2>&) {
    return false;
}

template<auto Name, auto... Names>
consteval std::size_t find_index() {
    std::size_t result = static_cast<std::size_t>(-1);
    std::size_t index = 0;
    auto f = [&]<auto n>() { 
        if (n == Name) result = index;
        ++index;
        return 0;
    };
    [[maybe_unused]] int unused[] = {f.template operator()<Names>()...};
        
    return result;
}

template<typename FieldType, ConstexprString FieldName>
struct Field {
    using Type = FieldType;
    static constexpr inline auto Name = FieldName;
};

template<typename T>
struct IsField : public std::false_type {};

template<typename T, ConstexprString Name>
struct IsField<Field<T, Name>> : public std::true_type {};

template<typename... Fields>
    requires std::conjunction_v<IsField<Fields>...>
struct SmartStruct {
protected:
    std::tuple<typename Fields::Type...> values;

public:
    template<ConstexprString Name>
    constexpr auto &get() {
        constexpr auto index = find_index<Name, (Fields::Name)...>();
        return std::get<index>(values);
    }

    template<ConstexprString Name>
    constexpr const auto &get() const {
        constexpr auto index = find_index<Name, (Fields::Name)...>();
        return std::get<index>(values);
    }
};

} // namespace SK

#endif // __SK_UTILITIES_SMART_STRUCT_H__
