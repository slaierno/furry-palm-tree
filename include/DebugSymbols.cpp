#include "DebugSymbols.hpp"
#include <iostream>
#include <fstream>

DebugSymbols::DebugSymbols(const std::string& in_filename) {
    deserialize(in_filename);
}

void DebugSymbols::deserialize(const std::string& in_filename) {
    std::ifstream in_file;
    in_file.open(in_filename);
    while(in_file.tellg() != std::iostream::pos_type(-1)) {
        DebugSymbol dbg_s;
        in_file >> dbg_s.address >> dbg_s.file >> dbg_s.line;
        push_back(dbg_s);
    }
    in_file.close();
}

void DebugSymbols::serialize(std::ofstream& out_file) {
    for(const auto& dbg_s : mSymbols) {
        out_file << dbg_s.address << dbg_s.file << dbg_s.line;
    }
}