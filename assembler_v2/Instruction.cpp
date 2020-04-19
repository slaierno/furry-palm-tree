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
    auto it = mTokenDeque.begin();
    while(it != mTokenDeque.end() && it->getType() == TokenType::Label) {
        const auto& label_str = it->get<std::string>();
        if(address != 0 && label_map[label_str] != 0) {
            throw std::logic_error("Duplicate label " + label_str + ".\n");
        }
        label_map[label_str] = address;
        label_present = true;
        ++it;
    }
    mTokenDeque.erase(mTokenDeque.begin(), it);
    return label_present;
}

#if 0
// OLD VERSION
// This gives a segfault...don't know why

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
#endif

constexpr cx::map<OP::Type, uint16_t, 8> br_to_cc {
    {OP::BR,    0b111},
    {OP::BRn,   0b100},
    {OP::BRz,   0b010},
    {OP::BRp,   0b001},
    {OP::BRnz,  0b110},
    {OP::BRnp,  0b101},
    {OP::BRzp,  0b011},
    {OP::BRnzp, 0b111},
};

constexpr cx::map<OP::Type, uint16_t, 6> trap_to_vec {
    {OP::GETC,  0x20},
    {OP::OUT,   0x21},
    {OP::PUTS,  0x22},
    {OP::IN,    0x23},
    {OP::PUTSP, 0x24},
    {OP::HALT,  0x25},
};

template <const OP::Type op> uint16_t buildInstruction(const Instruction& tokens, const LabelMap& label_map) {
    constexpr uint64_t opbit = 1LL << op;
    constexpr uint16_t opcode = opEnumToOpcodeMap[op];

    if constexpr (0x3F0000000 & opbit) return opcode << 12 | trap_to_vec[op];
    else {
        uint16_t inst = opcode << 12;
        [[maybe_unused]] uint16_t r1 = 0, r2 = 0;
        [[maybe_unused]] uint16_t off_bits;
        [[maybe_unused]] constexpr auto get_num = [](const Token& tkn, const unsigned n) {
            return tkn.get<int>() & 0xFFFF >> (16 - n);
        };
        if constexpr (0x0000800 & opbit) off_bits = 16;
        if constexpr (0x0000004 & opbit) off_bits = 11;
        if constexpr (0x007C7F8 & opbit) off_bits = 9;
        if constexpr (0x0C00000 & opbit) off_bits = 6;
        if constexpr (0x8300000 & opbit) off_bits = 5;
        if constexpr (0x7000000 & opbit) off_bits = 4;
        if constexpr (0x8300000 & opbit) if (tokens.back().getType() == TokenType::Number) inst |= 1 << 5; 
        if constexpr (0x2000000 & opbit) inst |= 0b01 << 4;
        if constexpr (0x4000000 & opbit) inst |= 0b11 << 4;
        if constexpr (0x0080000 & opbit) inst |= 0x3F;
        if constexpr (0xFFFC000 & opbit) r1 = get_num(tokens[1], 3); 
        if constexpr (0x00007F8 & opbit) r1 = br_to_cc[op]; 
        if constexpr (0x0000004 & opbit) r1 = 4;
        if constexpr (0x0000001 & opbit) r2 = 7;
        if constexpr (0xFF80000 & opbit) r2 = get_num(tokens[2], 3); 
        if constexpr (0x0003000 & opbit) r2 = get_num(tokens[1], 3); 
        if constexpr (0xFFFF7FD & opbit) inst |= r1 << 9 | r2 << 6;
        if constexpr (0x0000FFC & opbit) inst |= get_num(tokens[1], off_bits);
        if constexpr (0x007C000 & opbit) inst |= get_num(tokens[2], off_bits);
        if constexpr (0xFF00000 & opbit) inst |= get_num(tokens[3], off_bits);
        return inst;
    }
}

#define ENUM_MACRO(X) buildInstruction<OP::X>,
uint16_t (*inst_table[OP::COUNT])(const Instruction&, const LabelMap&) = {
    OP_TYPES
};
#undef ENUM_MACRO

std::vector<uint16_t> Instruction::GetMachineCode(const LabelMap& label_map) const {
    if (empty()) return std::vector<uint16_t>();
    switch (front().getType()) {
    case TokenType::Label: {
        //remove the label and call function again
        //TODO this should not happen...
        Instruction inst_cpy = *this;
        LabelMap label_map_cpy = label_map;
        inst_cpy.ConsumeLabels(label_map_cpy);
        //label_map_cpy gets discarded, we need to use the one provided
        //as an argument and keep any undefined label error
        return inst_cpy.GetMachineCode(label_map);
    }
    case TokenType::Instruction: {
        auto opcode = front().get<OP::Type>();
        uint16_t inst = inst_table[opcode](*this, label_map);
        return std::vector<uint16_t>{inst};
    }
    case TokenType::PseudoOp:
        switch(front().get<POP::Type>()) {
        case POP::FILL:
        case POP::BLKW:
            return std::vector<uint16_t>((*this)[1].get<int>(), {back().get<uint16_t>()});
        case POP::STRINGZ:
            return std::vector<uint16_t>(back().get<std::string>().begin(), back().get<std::string>().end());
        case POP::ORIG:
        case POP::END:
            return std::vector<uint16_t>();
        default:
            //TODO more meaningful error
            throw std::logic_error("Unexpected!\n");
        }
    default:
        throw std::logic_error("ERROR unknown error");
    }
}

uint16_t Instruction::GetAddressIncrement() const {
    switch (front().getType()) {
    case TokenType::PseudoOp:
        switch(front().get<POP::Type>()) {
        case POP::STRINGZ:
            return back().get<std::string>().length() + 1; //remember the NUL character
        case POP::BLKW:
            return (*this)[1].get<int>();
        case POP::ORIG: case POP::END:
            return 0;
        default:
            return 1;
        }
    case TokenType::Instruction:
        return 1;
    default:
        return 0; //TODO throw here
    }
}