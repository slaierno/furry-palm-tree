#include "Instruction.hpp"

Instruction::Instruction(std::string str, unsigned line_number) : mString(str), mLineNumber(line_number) {
    str = trim_line(str);

    // If first char is a quotes char, search for the enclosing quotes instead of first whitespace char
    auto find_end_pos = [&str] () {
        return (str.front() == '"') ? 
                str.find_first_of('"', 1) + 1 : 
                str.find_first_of(whitespace); 
    };
    for(size_t next_pos = 0;
                next_pos != std::string::npos;
                next_pos = str.find_first_not_of(whitespace, find_end_pos())) {
        str = str.substr(next_pos);
        if(auto token_str = str.substr(0, find_end_pos());
                !token_str.empty())
            mTokenDeque.emplace_back(token_str);
    }
}

bool Instruction::ConsumeLabels(LabelMap& label_map, uint16_t address) {
    bool label_present = false;
    for (auto& label = front(); label.getType() == TokenType::Label; label = front()) {
        const auto& label_str = label.get<std::string>();
        if(address != 0 && label_map[label_str] != 0) {
            throw std::logic_error("Duplicate label " + label_str + ".\n");
        }
        label_map[label_str] = address;
        pop_front();
        label_present = true;
    }
    return label_present;
}

uint16_t Instruction::GetMachineCode(const LabelMap& label_map) {
    size_t i = 0;
    while ((*this)[i].getType() == TokenType::Label) {
        const auto& tkn = (*this)[i];
        if (const auto& label_str = tkn.get<std::string>(); label_map.find(label_str) == label_map.end() || label_map.at(label_str) == 0) {
            throw std::logic_error("Undefined label " + label_str + ".\n");
        }
    }
    for(; i < size(); i++) {
        
    }
}