#pragma once
#include <cassert>
#include <deque>
#include "Token.hpp"
#include "commons.hpp"

class Instruction {
    std::deque<Token> mTokenDeque;
    std::string mString;
    unsigned mLineNumber;
    uint16_t mInstAddress = 0;
    size_t mFirstNonLabelIndex = std::numeric_limits<size_t>::max();

public:
    Instruction(std::string str, unsigned line_number = 0);

    std::vector<uint16_t> getMachineCode(const LabelMap&) const;

    auto getLineNumber() const { return mLineNumber; }
    auto   getOGString() const { return mString; }
    auto    getAddress() const { return mInstAddress; };

    /* Fills existing label in LabelMap with address. */
    bool fillLabelMap(LabelMap&, uint16_t) const;
    /* Put non-existing label in LabelMap. Address is set at 0. */
    bool fillLabelMap(LabelMap& map) const { return fillLabelMap(map, 0); };
    uint16_t setAddress(uint16_t);

          auto  empty() const { return mTokenDeque.empty(); }
          auto   size() const { return mTokenDeque.size();  }
          auto& front()       { return mTokenDeque.front(); }
    const auto& front() const { return mTokenDeque.front(); }
          auto&  back()       { return mTokenDeque.back();  }
    const auto&  back() const { return mTokenDeque.back();  }

                            auto    pop_front()           { return mTokenDeque.pop_front(); }
    template<typename ...T> auto    push_back(T... types) { return mTokenDeque.push_back(types...); }
    template<typename ...T> auto emplace_back(T... types) { return mTokenDeque.emplace_back(types...); }

    template<typename T> const auto& operator[](T idx) const { return mTokenDeque[idx]; }
    template<typename T>       auto& operator[](T idx)       { return mTokenDeque[idx]; }

    /* "Reduced" versions of existing methods
     * Every method works like there are no leading labels in mTokenDeque
     */
          auto   rsize() const { return mTokenDeque.size() - mFirstNonLabelIndex;  }
          auto  rempty() const { return rsize() == 0; }
          auto& rfront()       { return mTokenDeque[mFirstNonLabelIndex]; }
    const auto& rfront() const { return mTokenDeque[mFirstNonLabelIndex]; }
          auto&  rback()       { return mTokenDeque.back(); }
    const auto&  rback() const { return mTokenDeque.back(); }

    template<typename T> const auto& rget(T idx) const   { return mTokenDeque[idx+mFirstNonLabelIndex]; }
    template<typename T>       auto& rget(T idx)         { return mTokenDeque[idx+mFirstNonLabelIndex]; }
};