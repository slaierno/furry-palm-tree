#pragma once

#include <array>
#include <cstddef>
#include <string_view>
#include "cx_vector.hpp"

namespace cx
{
    struct static_string
    {
        std::size_t m_size{0};
        const char *m_data = nullptr;

        template <std::size_t N>
        constexpr static_string(const char (&str)[N])
        : m_size(N-1), m_data(&str[0]) {}
        constexpr static_string(const char* str, std::size_t s)
        : m_size(s), m_data(str) {}
        constexpr static_string() = default;

        constexpr     size_t   size() const { return m_size; }
        constexpr const char* c_str() const { return m_data; }
        constexpr const char* begin() const { return m_data; }
        constexpr const char*   end() const { return m_data + m_size; }
    };

    constexpr bool operator==(const static_string &x, const static_string &y)
    {
        return cx::equal(x.begin(), x.end(), y.begin(), y.end());
    }

    template<typename CharType, size_t Size>
    struct basic_string : vector<CharType, Size>
    {
        constexpr basic_string(const static_string &s) 
        : vector<CharType, Size>(s.begin(), s.end()) {}
        constexpr basic_string(const std::string_view &s)
        : vector<CharType, Size>(s.cbegin(), s.cend()) {}
        constexpr basic_string() = default;

        template<typename S> requires std::is_same_v<S, std::string_view> || std::is_same_v<S, static_string>
        constexpr basic_string &operator=(const S &s) { return *this = basic_string(s); }


        constexpr       auto length() const { return this->size(); }
        constexpr const char *c_str() const { return this->data(); }

        operator std::string() const { return std::string(c_str());}
    };

    template<typename CharType, size_t Size>
    constexpr bool operator==(const basic_string<CharType, Size> &lhs, const static_string &rhs)
    {
        return cx::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    using string = basic_string<char, 32>;
}