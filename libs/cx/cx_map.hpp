#pragma once

#include "cx_vector.hpp"

namespace cx {
    //TODO custom comparator
    template<typename Key, typename Value, size_t Size = 4, typename Compare = std::equal_to<Key>>
    class map {
        cx::vector<std::pair<Key, Value>, Size> m_data {};
        template<typename It>
        constexpr map(It begin, const It &end) {
            while (begin != end) {
                const auto it = find(begin->first);
                if (it != m_data.end()) throw std::logic_error("Duplicate key");
                m_data.push_back(*begin);
                ++begin;
            }
        }
    public:
        constexpr map(std::initializer_list<std::pair<Key, Value>> il)
            : map(il.begin(), il.end()) {};
        constexpr map() = default;

        constexpr auto cbegin() const { return m_data.cbegin(); }
        constexpr auto  begin() const { return m_data.begin(); }
        constexpr auto  begin()       { return m_data.begin(); }
        constexpr auto   cend() const { return m_data.cend(); }
        constexpr auto    end() const { return m_data.end(); }
        constexpr auto    end()       { return m_data.end(); }

        constexpr auto capacity() const { return Size; }
        constexpr auto     size() const { return m_data.size(); }
        constexpr auto    empty() const { return size() == 0; }

        constexpr auto find(const Key& k) const { return find_impl(*this, k); }
        constexpr auto find(const Key& k)       { return find_impl(*this, k); }

        constexpr auto insert(Key k, Value v) {
            if (find(k) != m_data.end()) throw std::logic_error("Duplicate key");
            m_data.push_back({k, v});
        }

        constexpr const Value &operator[](const Key &k) const {
            const auto it = find(k);
            if (it == end()) {
                throw std::logic_error("Key not found");
            } else {
                return it->second;
            }
        }

    private:
        template <typename This>
        static constexpr auto find_impl(This &&t, const Key &k)
        {
        return cx::find_if(t.begin(), t.end(),
                        [&k] (const auto &d) { return Compare{}(d.first, k); });
        }
    };
}