#pragma once

#include "cx_algorithm.hpp"

namespace cx {    
    template<typename T, size_t Size = 4>
    class vector {
        using storage_t = std::array<T, Size>;
        storage_t m_data {};
           size_t m_size {0};
    public:
        using value_type = T;
        using iterator        = typename storage_t::iterator;
        using const_iterator  = typename storage_t::const_iterator;
        using reference       = typename storage_t::reference;
        using const_reference = typename storage_t::const_reference;

        template<typename It>
        constexpr vector(It begin, const It &end) { for (;begin != end; push_back(*(begin++))); }
        constexpr vector(std::initializer_list<T> il) : vector(il.begin(), il.end()) {};
        constexpr vector() = default;

        constexpr auto cbegin() const { return m_data.begin(); }
        constexpr auto  begin() const { return m_data.begin(); }
        constexpr auto  begin()       { return m_data.begin(); }
        constexpr auto   cend() const { return std::next(m_data.begin(), m_size); }
        constexpr auto    end() const { return std::next(m_data.begin(), m_size); }
        constexpr auto    end()       { return std::next(m_data.begin(), m_size); }

        constexpr T& push_back(T t_v) {
            if (m_size >= Size) throw std::range_error("Index past end of vector");
            else {
                T& v = m_data[m_size++];
                v = std::move(t_v);
                return v;
            }
        }
        constexpr const T &back() const {
            if (empty()) throw std::range_error("Index past end of vector");
            else return m_data[m_size - 1];
        }
        constexpr T &back() {
            if (empty()) throw std::range_error("Index past end of vector");
            else return m_data[m_size - 1];
        }

        constexpr auto capacity() const { return Size; }
        constexpr auto     size() const { return m_size; }
        constexpr auto    empty() const { return m_size == 0; }
        constexpr const T* data() const { return m_data.data(); }

        constexpr void    clear() { m_size = 0; }
    };
    
    template<typename T, size_t Size1, size_t Size2>
    constexpr bool operator==(const vector<T, Size1> &x, const vector<T, Size2> &y)
    {
        return cx::equal(x.begin(), x.end(), y.begin(), y.end());
    }
}