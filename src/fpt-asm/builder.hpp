#pragma once

#include <stdint.h>
#include "Token.hpp"
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
            R2 = T[1], 
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
        off_bits = 5  -> ADD, AND, XOR
        off_bits = 4  -> LSHF, RSHFL, RSHFA
        R1 = T[1]     -> LD, ST, LDI, STI, LEA, NOT, XOR, ADD, AND, LDR, STR, LSHF, RSHFL, RSHFA
        R1 = CF       -> BR, BRn, BRz, BRp, BRnz, BRnp, BRzp, BRnzp
        R1 = 4        -> JSR
        R2 = 7        -> RET
        R2 = T[2]     -> NOT, XOR, ADD, AND, LDR, STR, LSHF, RSHFL, RSHFA
        R2 = T[1]     -> JSRR, JMP
        set_regs      -> RET, JSR, BR, BRn, BRz, BRp, BRnz, BRnp, BRzp, BRnzp, JSRR, JMP, LD, ST, LDI, STI, LEA, XOR, NOT, ADD, AND, LDR, STR, LSHF, RSHFL, RSHFA
        set_off(T[1]) -> JSR, BR, BRn, BRz, BRp, BRnz, BRnp, BRzp, BRnzp, TRAP
        set_off(T[2]) -> LD, ST, LDI, STI, LEA
        I[5-0] = 1    -> NOT
        set_off(T[3]) -> ADD, AND, XOR, LDR, STR, LSHF, RSHFL, RSHFA
        LF = 0b10     -> ANDi, ADDi, XORi
             0b01     -> RSHFL
             0b11     -> RSHFA
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
    if(0x00000004 & opbit) off_bits = 11;
    if(0x0007C7F8 & opbit) off_bits = 9;
    if(0x00C00000 & opbit) off_bits = 6;
    if(0x10300000 & opbit) off_bits = 5;
    if(0x07000000 & opbit) off_bits = 4;
    if(0x10300000 & opbit && (tokens.back().isNumber())) inst |= 1 << 5; 
    if(0x02000000 & opbit) inst |= 0b01 << 4;
    if(0x04000000 & opbit) inst |= 0b11 << 4;
    if(0x00080000 & opbit) inst |= 0x3F;
    if(0x17FFC000 & opbit) r1 = tokens[1].getNumValue(3); 
    if(0x000007F8 & opbit) r1 = tokens[0].getCondFlags(); 
    if(0x00000004 & opbit) r1 = 4;
    if(0x00000001 & opbit) r2 = 7;
    if(0x17F80000 & opbit) r2 = tokens[2].getNumValue(3); 
    if(0x00003000 & opbit) r2 = tokens[1].getNumValue(3); 
    if(0x17FFF7FD & opbit) inst |= r1 << 9 | r2 << 6;
    if(0x00000FFC & opbit) inst |= tokens[1].getNumValue(off_bits);
    if(0x0007C000 & opbit) inst |= tokens[2].getNumValue(off_bits);
    if(0x17F00000 & opbit) inst |= tokens[3].getNumValue(off_bits);
    return inst;
}
#pragma GCC diagnostic pop

/* Instruction 28 is absent because TRAP has been disabled */
uint16_t (*inst_table[OP::COUNT])(const TokenList&) = {
    buildInstruction<0>,  buildInstruction<1>,  buildInstruction<2>,  buildInstruction<3>,
    buildInstruction<4>,  buildInstruction<5>,  buildInstruction<6>,  buildInstruction<7>,
    buildInstruction<5>,  buildInstruction<9>,  buildInstruction<10>, buildInstruction<11>,
    buildInstruction<12>, buildInstruction<13>, buildInstruction<14>, buildInstruction<15>,
    buildInstruction<16>, buildInstruction<17>, buildInstruction<18>, buildInstruction<19>,
    buildInstruction<20>, buildInstruction<21>, buildInstruction<22>, buildInstruction<23>,
    buildInstruction<24>, buildInstruction<25>, buildInstruction<26>, buildInstruction<28>
};

/* Modify token_list to obtain a TRAP trapvector8 instruction */
void trapToInstruction(TokenList& token_list) {
    token_list = {
        Token(TokenType::Instruction, OP::TRAP),
        Token(TokenType::HexNumber  , (int)trapEnumToOpcodeMap[token_list.front().get<TRAP::Type>()]),
    };
}

uint16_t assembleLine(TokenList token_list, std::string& ret_string) {
    if (token_list.empty()) return 0;
    auto front = [&token_list]() {return token_list.front();};
    auto back = [&token_list]() {return token_list.back();};

    switch (front().getType()) {
    case TokenType::Label:
        //remove the label and call function again
        token_list.pop_front();
        return assembleLine(token_list, ret_string);
    case TokenType::Trap:
        trapToInstruction(token_list);
        // intended fallthrough
    case TokenType::Instruction: {
        updateInstructionAddress();
        auto opcode = front().get<OP::Type>();
        uint16_t inst = inst_table[opcode](token_list);
        return inst;
    }
    case TokenType::PseudoOp:
        switch(front().get<POP::Type>()) {
        case POP::FILL:
            updateInstructionAddress();
            return back().getNumValue();
        case POP::BLKW:
            updateInstructionAddress(back().getNumValue());
            return 0;
        case POP::STRINGZ:
            ret_string = back().get<std::string>();
            updateInstructionAddress(ret_string.length() + 1); 
            return 0;
        case POP::ORIG:
        case POP::END:
            return 0;
        default:
            //TODO more meaningful error
            throw asm_error::invalid_pseudo_op();
        }
    default:
        throw std::logic_error("ERROR unknown error");
    }
}
