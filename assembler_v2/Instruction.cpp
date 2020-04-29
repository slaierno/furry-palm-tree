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
        if(auto token_str = str.substr(0, find_end_pos()); !token_str.empty()) {
            if (const auto& back = mTokenDeque.emplace_back(token_str); back.getType() != TokenType::Label) {
                mFirstNonLabelIndex = std::min(mFirstNonLabelIndex, mTokenDeque.size() - 1);
            }
        }
    }
    if(mTokenDeque.empty()) mFirstNonLabelIndex = 0;
}

bool Instruction::fillLabelMap(LabelMap& label_map, uint16_t address) const {
    bool label_present = false;
    for(auto it = mTokenDeque.begin();
        it != std::next(mTokenDeque.begin(), mFirstNonLabelIndex);
        it++) {
        const auto& label_str = it->get<cx::string>();
        if(address != 0 && label_map[label_str] != 0) {
            throw std::logic_error("Duplicate label " + std::string(label_str) + ".\n");
        }
        label_map[label_str] = address;
        label_present = true;
    }
    return label_present;
}

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

template <const OP::Type op> uint16_t build_instruction(const Instruction& tokens, const LabelMap& label_map) {
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
        if constexpr (0x0000800 & opbit) inst |= get_num(tokens.rget(1), 16); //TRAP
        if constexpr (0x0000004 & opbit) off_bits = 11;
        if constexpr (0x007C7F8 & opbit) off_bits = 9;
        if constexpr (0x0C00000 & opbit) off_bits = 6;
        if constexpr (0x8300000 & opbit) off_bits = 5;
        if constexpr (0x7000000 & opbit) off_bits = 4;
        if constexpr (0x8300000 & opbit) if (tokens.rget(3).getType() == TokenType::Number) inst |= 1 << 5;
        if constexpr (0x2000000 & opbit) inst |= 0b01 << 4;
        if constexpr (0x4000000 & opbit) inst |= 0b11 << 4;
        if constexpr (0x0080000 & opbit) inst |= 0x3F;
        if constexpr (0xFFFC000 & opbit) r1 = get_num(tokens.rget(1), 3);
        if constexpr (0x00007F8 & opbit) r1 = br_to_cc[op]; 
        if constexpr (0x0000004 & opbit) r1 = 4;
        if constexpr (0x0000001 & opbit) r2 = 7;
        if constexpr (0xFF80000 & opbit) r2 = get_num(tokens.rget(2), 3);
        if constexpr (0x0003000 & opbit) r2 = get_num(tokens.rget(1), 3);
        if constexpr (0xFFFF7FD & opbit) inst |= r1 << 9 | r2 << 6;
        if constexpr (0x007C7FC & opbit) {
            if (const auto& it = label_map.find(tokens.rback().getString());
                            it != label_map.end() && it->second != 0) {
                int16_t label_off = (int16_t)(it->second - tokens.getAddress()),
                              min = -(1 << (off_bits - 1)),
                              max =  (1 << (off_bits - 1)) - 1;
                if(label_off < min || label_off > max)
                    throw std::logic_error("Label " + it->first + " too far!");
                return label_off & 0xFFFF >> (16 - off_bits);
            } else {
                throw std::logic_error("Label " + it->first + " not found");
            }
        }
        if constexpr (0xFF00000 & opbit) inst |= get_num(tokens[3], off_bits);
        return inst;
    }
}

#define ENUM_MACRO(X) build_instruction<OP::X>,
uint16_t (*inst_table[OP::COUNT])(const Instruction&, const LabelMap&) = {
    OP_TYPES
};
#undef ENUM_MACRO

std::vector<uint16_t> Instruction::getMachineCode(const LabelMap& label_map) const {
    if (empty()) return std::vector<uint16_t>();
    switch (rfront().getType()) {
    case TokenType::Instruction: {
        auto opcode = rfront().get<OP::Type>();
        uint16_t inst = inst_table[opcode](*this, label_map);
        return std::vector<uint16_t>{inst};
    }
    case TokenType::PseudoOp:
        switch(rfront().get<POP::Type>()) {
        case POP::FILL:
        case POP::BLKW:
            return std::vector<uint16_t>((*this)[1].get<int>(), {back().get<uint16_t>()});
        case POP::STRINGZ:
            return std::vector<uint16_t>(back().get<cx::string>().begin(), back().get<cx::string>().end());
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

uint16_t Instruction::setAddress(uint16_t address) {
    switch (rfront().getType()) {
    case TokenType::PseudoOp:
        switch(rfront().get<POP::Type>()) {
        case POP::STRINGZ:
            address += back().get<cx::string>().length() + 1; //remember the NUL character
            break;
        case POP::BLKW:
            address += (*this)[1].get<int>();
            break;
        case POP::FILL:
            address++;
            break;
        default:;
        }
        break;
    case TokenType::Instruction:
        address++;
        break;
    default:
        throw std::logic_error("Unexpected"); //TODO throw here
    }
    return mInstAddress = address;
}