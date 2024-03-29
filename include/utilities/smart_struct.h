#ifndef __SK_UTILITIES_SMART_STRUCT_H__
#define __SK_UTILITIES_SMART_STRUCT_H__

#include <utilities/consteval_string.h>

#include <algorithm>
#include <concepts>
#include <cstddef>  // std::size_t
#include <string_view>
#include <tuple>
#include <type_traits>

namespace SK {

template<typename FieldType, ConstevalString FieldName>
struct Field {
    using Type = FieldType;
    static constexpr auto Name = FieldName;
};

namespace detail {

/* Checks if there are no two values that are the same among the passed ConstevalStrings */
consteval bool distinct_field_names(auto ...names) {
    constexpr std::size_t size = sizeof...(names);
    if constexpr (size == 0) {
        return true;
    } else {
        std::string_view arr[size] = { std::string_view(names.content)... };
        std::sort(std::begin(arr), std::end(arr));
        const auto it_end = std::end(arr);
        return it_end == std::unique(std::begin(arr), std::end(arr));
    }
}

template<auto Name, auto... Names>
consteval std::size_t find_index() {
    std::size_t result = static_cast<std::size_t>(-1);
    std::size_t index = 0;

    /* Loop over the names and find the index corresponding to Name */
    ([&]<auto Name_>() {
        if (Name_ == Name)
            result = index;
        ++index;
    }.template operator()<Names>(), ...);
        
    return result;
}

template<typename T>
struct IsField : public std::false_type {};

template<typename T, ConstevalString Name>
struct IsField<Field<T, Name>> : public std::true_type {};

} // namespace detail

template<typename... Fields>
    requires std::conjunction_v<detail::IsField<Fields>...> &&
             (detail::distinct_field_names((Fields::Name)...))
struct SmartStruct {
protected:
    std::tuple<typename Fields::Type...> values;

public:
    template<ConstevalString Name>
    constexpr auto &get() {
        constexpr auto index = detail::find_index<Name, (Fields::Name)...>();
        return std::get<index>(values);
    }

    template<ConstevalString Name>
    constexpr const auto &get() const {
        constexpr auto index = detail::find_index<Name, (Fields::Name)...>();
        return std::get<index>(values);
    }
};

} // namespace SK

#endif // __SK_UTILITIES_SMART_STRUCT_H__
