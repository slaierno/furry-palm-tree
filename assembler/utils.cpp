#include "utils.hpp"
#include "errors.hpp"

TokenList tokenize(std::string line) {
    static const std::string delims = " ,\n";
    static const std::string comm = ";";

    /* Remove comments */
    size_t comment_pos = line.find_first_of(comm);
    if(comment_pos != std::string::npos) {
        line = line.substr(0, comment_pos) + "\n"; //Reappend newline
    }

    /* Extract tokens */
    TokenList token_list;
    size_t del_pos = 0, text_pos = 0;
    while(true) {
        /* text_pos can be npos in case of blank or comment-only lines */
        size_t text_pos = line.find_first_not_of(delims, del_pos);
        if(text_pos == std::string::npos) {
            return token_list;
        }
        /* del_pos cannot be npos since there is _at least_ the newline at the end */
        del_pos = line.find_first_of(delims, text_pos);
        token_list.push_back(line.substr(text_pos, del_pos - text_pos));
    }
}

void validationStep(TokenList tokens) {
    switch(tokens[0].mType) {
        case TokenType::Instruction: {
            /* Construct argument vector */
            std::vector<enum TokenType> args_type;
            for(auto& token : tokens) {
                if(token.mType == TokenType::Instruction) continue; //skip first element
                args_type.push_back(token.mType);
            }
            /* Find if argument sequence is valid */
            auto range = validation_map.equal_range(tokens[0].get<enum OP>());
            for (auto i = range.first; i != range.second; ++i) {
                if(i->second == args_type) {
                    /* Found a match! */
                    /* Now check if int values are valid */
                    switch(tokens[0].get<enum OP>()) {
                        case OP::ADD:
                        case OP::AND:
                            if(tokens[2].isNumber()) checkBitRange(tokens[2], 5);
                            break;
                        case OP::LD:
                        case OP::LDI:
                        case OP::LEA:
                        case OP::ST:
                        case OP::STI:
                            checkBitRange(tokens[1], 9);
                            break;
                        case OP::LDR:
                        case OP::STR:
                            checkBitRange(tokens[1], 6);
                            break;
                        case OP::LSHF:
                        case OP::RSHFL:
                        case OP::RSHFA:
                            checkBitRangeUnsigned(tokens[1], 4);
                            break;
                        case OP::TRAP:
                            checkBitRangeUnsigned(tokens[1], 8);
                            break;
                    }

                    return;
                }
            }
            throw asm_error::invalid_format(tokens);
        } break;
        case TokenType::Label: {
            if (tokens.size() > 1)
                throw asm_error::invalid_label_decl();
            if (label_map.find(tokens[0].get<std::string>()) != label_map.end())
                throw asm_error::duplicate_label(tokens[0]);
            label_map.insert(std::pair(tokens[0].get<std::string>(), inst_address));
        }
        break;
        default: {
            /* Only one token which may be a reserved one used as a label */
            if (tokens.size() == 1) {
                throw asm_error::invalid_label_name(tokens[0]);
            } else {
                // TODO miscellaneous
                throw asm_error::invalid_token();
            }
        } break;
    }
}

/*
    Common functions:

        set_regs() := inst |= r1 << 9 | r2 << 6;
        set_off(x) := x.getNumValue(off_bits);
            CF  := tokens[0].getCondFlags();

    Instruction op-per-op:

        RTI, 
            RETURN
        RET, 
            R2 = 7, 
            set_regs()
        JSR, 
            R1 = 4, 
            off_bits = 11, 
            set_off(T[1]),
            set_regs()
        BR, BRn, BRz, BRp, BRnz, BRnp, BRzp, BRnzp, 
            R1 = CF, 
            off_bits = 9, 
            set_off(T[1]), 
            set_regs()
        TRAP,
            off_bits = 16, 
            set_off(T[1])
        JSRR, JMP, 
            R1 = T[1], 
            set_regs()
        LD, ST, LDI, STI, LEA,
            R1 = T[1], 
            off_bits = 9, 
            set_off(T[2]),
            set_regs(), 
        NOT, 
            R1 = T[1], 
            R2 = T[2], 
            set_regs(),
            inst |= 0x3F
        ADD, AND, 
            off_bits = 5, 
            R1 = T[1], 
            R2 = T[2], 
            if (T[3].type == num) 
                inst |= 1 << 5,
            set_off(T[3]), 
            set_regs(),
        LDR, STR, 
            off_bits = 6, 
            R1 = T[1], 
            R2 = T[2], 
            set_off(T[3])
            set_regs()
        LSHF, RSHFL, RSHFA, 
            off_bits = 4, 
            R1 = T[1], 
            R2 = T[2], 
            if(op == RSHFL)
                inst |= 0b1 << 5
            if(op == RSHFA)
                inst |= 0b11 << 4
            set_off(T[3]), 
            set_regs()

    Instruction groups:

        off_bits = 16 -> TRAP
        off_bits = 11 -> JSR
        off_bits = 9  -> BR, BRn, BRz, BRp, BRnz, BRnp, BRzp, BRnzp, LD, ST, LDI, STI, LEA
        off_bist = 6  -> LDR, STR
        off_bits = 5  -> ADD, AND
        off_bits = 4  -> LSHF, RSHFL, RSHFA
        R1 = T[1]     -> JSRR, JMP, LD, ST, LDI, STI, LEA, NOT, ADD, AND, LDR, STR, LSHF, RSHFL, RSHFA
        R1 = CF       -> BR, BRn, BRz, BRp, BRnz, BRnp, BRzp, BRnzp
        R1 = 4        -> JSR
        R2 = 7        -> RET
        R2 = T[2]     -> NOT, ADD, AND, LDR, STR, LSHF, RSHFL, RSHFA
        set_regs      -> RET, JSR, BR, BRn, BRz, BRp, BRnz, BRnp, BRzp, BRnzp, JSRR, JMP, LD, ST, LDI, STI, LEA, NOT, ADD, AND, LDR, STR, LSHF, RSHFL, RSHFA
        set_off(T[1]) -> JSR, BR, BRn, BRz, BRp, BRnz, BRnp, BRzp, BRnzp, TRAP
        set_off(T[2]) -> LD, ST, LDI, STI, LEA
        I[5-0] = 1    -> NOT
        set_off(T[3]) -> ADD, AND, LDR, STR, LSHF, RSHFL, RSHFA
        I[5]   = 1    -> ADDi, ANDi, RSHFL
        I[5-4] = 1    -> RSHFA

        LF = 0b10 -> ANDi, ADDi, RSHFL
            0b11 -> RSHFA
        CF = 0bnzp
*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsequence-point"
template <const unsigned op> uint16_t buildInstruction(const TokenList& tokens) {
    const uint32_t opbit = 1 << op;
    const uint16_t opcode = tokens[0].getNumValue(4);
    uint16_t inst = opcode << 12;
    uint16_t r1 = 0, r2 = 0;
    uint16_t off_bits = 16;
    // if(op & 0x0000800) off_bits = 16;
    if(0x0000004 & opbit) off_bits = 11;
    if(0x007C7F8 & opbit) off_bits = 9;
    if(0x0C00000 & opbit) off_bits = 6;
    if(0x0300000 & opbit) off_bits = 5;
    if(0x7000000 & opbit) off_bits = 4;
    if(0x2300000 & opbit && (tokens.back().isNumber())) inst |= 1 << 5; 
    if(0x4000000 & opbit) inst |= 0b11 << 4;
    if(0x0080000 & opbit) inst |= 0x3F;
    if(0x7FFF000 & opbit) r1 = tokens[1].getNumValue(); 
    if(0x00007F8 & opbit) r1 = tokens[0].getCondFlags(); 
    if(0x0000004 & opbit) r1 = 4;
    if(0x0000001 & opbit) r1 = 7;
    if(0x7F80000 & opbit) r2 = tokens[2].getNumValue(); 
    if(0x7FFF7FD & opbit) inst |= r1 << 9 | r2 << 6;
    if(0x0000FFC & opbit) inst |= tokens[1].getNumValue(off_bits);
    if(0x007C000 & opbit) inst |= tokens[2].getNumValue(off_bits);
    if(0x7F00000 & opbit) inst |= tokens[3].getNumValue(off_bits);
    return inst;
}
#pragma GCC diagnostic pop

uint16_t (*inst_table[OP::COUNT])(const TokenList&) = {
    buildInstruction<0>,  buildInstruction<1>,  buildInstruction<2>,  buildInstruction<3>,
    buildInstruction<4>,  buildInstruction<5>,  buildInstruction<6>,  buildInstruction<7>,
    buildInstruction<5>,  buildInstruction<9>,  buildInstruction<10>, buildInstruction<11>,
    buildInstruction<12>, buildInstruction<13>, buildInstruction<14>, buildInstruction<15>,
    buildInstruction<16>, buildInstruction<17>, buildInstruction<18>, buildInstruction<19>,
    buildInstruction<20>, buildInstruction<21>, buildInstruction<22>, buildInstruction<23>,
    buildInstruction<24>, buildInstruction<25>, buildInstruction<26>,
};

uint16_t inst_address = 0x3000;
std::map<std::string, uint16_t> label_map;

void validateLine(std::string& line) {
    TokenList token_list = tokenize(line);
    //TODO catch exception?
    validationStep(token_list);
    if(token_list[0].mType == TokenType::Instruction) 
        inst_address++;
}

uint16_t assembleLine(std::string& line) {
    TokenList token_list = tokenize(line);
    switch(token_list[0].mType) {
        case TokenType::Instruction: {
            auto opcode = token_list[0].get<enum OP>();
            uint16_t inst = inst_table[opcode](token_list);
            inst_address++;
            return inst;
        }
        case TokenType::Label:
            return 0;
        default:
            throw std::logic_error("ERROR unknown error");
    }
}

void checkBitRange(Token const& token, const int nBit) {
    if (nBit < 0 || nBit > 11) {
        throw std::logic_error("unexpected bit range, this is VERY bad");
    }
    int lower = -(1 << (nBit-1));
    int upper =  (1 << (nBit-1)) - 1;
    if (token.get<int>() < lower || token.get<int>() > upper)
        throw asm_error::out_of_range_integer(nBit);
}

void checkBitRangeUnsigned(Token const& token, const int nBit) {
    if (nBit < 0 || nBit > 8) {
        throw std::logic_error("unexpected bit range, this is VERY bad");
    }
    int lower = 0;
    int upper = (1 << nBit) - 1;
    if (token.get<int>() < lower || token.get<int>() > upper)
        throw asm_error::out_of_range_integer_unsigned(nBit);
}