#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

struct DebugSymbol {
    uint16_t address;
    std::filesystem::path file = "";
    unsigned line = 0;

    friend std::istream& operator>> (std::istream& is,       DebugSymbol& dbg_s) { return is >> dbg_s.address >> dbg_s.file >> dbg_s.line; }
    friend std::ostream& operator<< (std::ostream& os, const DebugSymbol& dbg_s) { return os << dbg_s.address << dbg_s.file << dbg_s.line; }
};

class DebugSymbols {
    std::vector<DebugSymbol> mSymbols;

    void deserialize(const std::string& in_filename) {
        std::ifstream in_file(in_filename);
        while(in_file.tellg() != std::iostream::pos_type(-1)) {
            in_file >> mSymbols.emplace_back();
        }
    }

public:
    DebugSymbols(const std::string& in_filename) { deserialize(in_filename); }
    DebugSymbols() {};
    template<typename ...T> auto push_back(T... types) { return mSymbols.push_back(types...); }
    template<typename ...T> auto emplace_back(T... types) { return mSymbols.emplace_back(types...); }

    void serialize(std::ofstream& out_file) { for(const auto& dbg_s : mSymbols) out_file << dbg_s; }

    const DebugSymbol& operator[](uint16_t address) const {
        auto offset = address - mSymbols[0].address;
        if (mSymbols[offset].address == address) {
            return mSymbols[offset];
        } else {
            //TODO this should not happen, right?
            auto lb = std::lower_bound(mSymbols.begin(), mSymbols.end(), DebugSymbol{address},
                [](const DebugSymbol& s1, const DebugSymbol& s2) {
                    return s1.address < s2.address;
            });
            //TODO meaningful error
            if (lb == mSymbols.end() || lb->address != address) throw std::logic_error("Address not found");
            return *lb;
        }
    }
};