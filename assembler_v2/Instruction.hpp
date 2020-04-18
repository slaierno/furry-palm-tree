#pragma once
#include <deque>
#include "Token.hpp"
#include "commons.hpp"

class Instruction {
    std::deque<Token> mTokenDeque;
    std::string mString;
    unsigned mLineNumber;
public:
    Instruction(std::string str = "", unsigned line_number = 0);

    /* Consume labels at the start of an instruction.
    * It returns a boolean if at least one label was present.
    * TODO remove bool return value?
    */
    bool ConsumeLabels(LabelMap&, uint16_t address = 0);

    uint16_t GetMachineCode(const LabelMap&) const;

    auto GetLineNumber() const { return mLineNumber; }
    auto GetOGString() const { return mString; }

    auto empty() const { return mTokenDeque.empty(); }
    auto size() const { return mTokenDeque.size(); }
    auto& front() { return mTokenDeque.front(); }
    const auto& front() const { return mTokenDeque.front(); }
    auto& back() { return mTokenDeque.back(); }
    const auto& back() const { return mTokenDeque.back(); }
    auto pop_front() { return mTokenDeque.pop_front(); }
    template<typename ...T> auto push_back(T... types) { return mTokenDeque.push_back(types...); }
    template<typename ...T> auto emplace_back(T... types) { return mTokenDeque.emplace_back(types...); }

    template<typename T> const auto& operator[](T idx) const { return mTokenDeque[idx]; }
    template<typename T> auto& operator[](T idx) { return mTokenDeque[idx]; }
};
