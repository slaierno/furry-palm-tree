#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct DebugSymbol {
    uint16_t address;
    std::string file;
    unsigned line;
};

class DebugSymbols {
    std::vector<DebugSymbol> mSymbols;
    void Deserialize(const std::ofstream& in_file);
public:
    DebugSymbols(const std::ofstream& in_file);
    DebugSymbols() {};
    template<typename ...T> auto push_back(T... types) { return mSymbols.push_back(types...); }
    template<typename ...T> auto emplace_back(T... types) { return mSymbols.emplace_back(types...); }
    void Serialize(std::ofstream& out_file);
};