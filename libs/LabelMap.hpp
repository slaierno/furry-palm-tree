#pragma once

#include <map>
#include <string>
#include <cstdint>

/* A LabelMap is a map where [key,value] == [label name, address location] */
using LabelMap = std::map<std::string, uint16_t>;

namespace fpt {
    inline void serialize(const LabelMap& label_map, std::ostream& os) {
        for(const auto& [label, address] : label_map) {
            os << label << address;
        }
    }
}