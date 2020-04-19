#pragma once

#include <array>
#include <stdexcept>
#include <variant>
#include <string>

namespace cx {    
    template<class InputIt, class UnaryPredicate>
    constexpr InputIt find_if(InputIt first, InputIt last, UnaryPredicate p)
    {
        for (; first != last; ++first) {
            if (p(*first)) {
                return first;
            }
        }
        return last;
    }

    template<typename T, size_t Size = 4>
    class vector {
        using storage_t = std::array<T, Size>;
        storage_t m_data {};
        size_t m_size{0};
    public:
        using iterator = typename storage_t::iterator;
        using const_iterator = typename storage_t::const_iterator;
        using value_type = T;
        using reference = typename storage_t::reference;
        using const_reference = typename storage_t::const_reference;

        template<typename It>
        constexpr vector(It begin, const It &end) {
            while (begin != end) {
                push_back(*begin);
                ++begin;
            }
        }
        constexpr vector(std::initializer_list<T> il)
            : vector(il.begin(), il.end()) {};
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

    struct string {
        size_t m_size{0};
        const char* m_data = nullptr;

        template<size_t N> 
        constexpr string(const char (&str)[N])
            : m_size(N-1), m_data(&str[0]) {}
        constexpr string(const char* str, size_t s)
            : m_size(s), m_data(str) {}
        constexpr string() = default;

        constexpr     size_t   size() const { return m_size; }
        constexpr const char *c_str() const { return m_data; }
        constexpr const char *begin() const { return m_data; }
        constexpr const char   *end() const { return m_data + m_size; }
                  std::string   str() const { return std::string(m_data); }
    };

    constexpr bool operator==(const cx::string &x, const cx::string &y) {
        auto it1 = x.begin(), it2 = y.begin();
        while(it1 != x.end() && it2 != y.end() && *it1 == *it2)
            it1++, it2++;
        return it1 == x.end() && it2 == y.end();
    }

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

    namespace {
        // template <class T, class U> struct is_same : std::false_type {};
        // template <class T>          struct is_same<T, T> : std::true_type {};
        // template <>                 struct is_same<cx::string, std::string> : std::true_type {};
        // template <>                 struct is_same<std::string, cx::string> : std::true_type {};
        // template <class T, class U> inline constexpr bool is_same_v = cx::is_same<T, U>::value;
        template <class T, class U> struct is_one_of;
        template <class T, class... Ts> 
        struct is_one_of<T, std::variant<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};
        template <class T, class V> struct holds_variant : is_one_of<T, V> {};
    }
    template <class T, class V> inline constexpr bool holds_variant_v = holds_variant<T, V>::value;
    template <class T> inline constexpr bool fail_v = !std::is_same_v<int, int>;
};