#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>

struct DebugSymbol {
    uint16_t address;
    std::filesystem::path file;
    unsigned line;
};

class DebugSymbols {
    std::vector<DebugSymbol> mSymbols;
    void deserialize(const std::string& in_file);
public:
    DebugSymbols(const std::string& in_filename);
    DebugSymbols() {};
    template<typename ...T> auto push_back(T... types) { return mSymbols.push_back(types...); }
    template<typename ...T> auto emplace_back(T... types) { return mSymbols.emplace_back(types...); }
    void serialize(std::ofstream& out_file);
};