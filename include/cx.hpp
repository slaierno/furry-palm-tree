#pragma once

#include <variant>

namespace cx {
    namespace {
        template <class T, class U> struct is_one_of;
        template <class T, class... Ts> 
        struct is_one_of<T, std::variant<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};
        template <class T, class V> struct holds_variant : is_one_of<T, V> {};
    }
    template <class T, class V> inline constexpr bool holds_variant_v = holds_variant<T, V>::value;
    template <class T> inline constexpr bool fail_v = !std::is_same_v<int, int>;
};